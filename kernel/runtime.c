#include <kernel/filesystem.h>
#include <driver/serial.h>
#include <gui/gnrtx.h>
#include <kernel/task.h>
#include <system.h>
#include <iolib.h>
#include <type.h>
#include <mem.h>


// global values for tasking
int g_return, g_argc;
char **g_argv;

uint32_t **lib_functions = 0;
int lib_count = 0;

void tasked_program() {
    uint8_t *binary_mem = task_get_bin_mem(task_get_current_pid());
    g_return = ((int (*)(int, char**)) binary_mem)(g_argc, g_argv);

    // fill memory with 0
    mem_set((uint8_t *) binary_mem, 0, mem_get_alloc_size((int) binary_mem));

    free(binary_mem);

    if (task_get_next_pid() == 0) clear_screen();
    task_kill_task_switch(task_get_next_pid());
}

int run_binary(char path[], int silence, int argc, char **argv) {
    // TODO: check if file is executable
    // TODO: check if there is enough memory

    serial_debug("RUNTIME", path);
    (void) silence;

    int pid = task_create(tasked_program, path);

    uint8_t *binary_mem = calloc(fs_get_file_size(path) * 126);
    uint32_t *file = fs_declare_read_array(path);
    fs_read_file(path, file);

    for (int i = 0; file[i] != (uint32_t) -1 ; i++)
        binary_mem[i] = (uint8_t) file[i] & 0xFF;

    free(file);

    g_argc = argc;
    g_argv = argv;
    g_return = 0;

    task_set_bin_mem(pid, binary_mem);

    task_switch(pid);

    // TODO: memory leak detection

    return g_return;
}

int run_ifexist(char path[], int argc, char **argv) {
    if (fs_does_path_exists(path) && fs_type_sector(fs_path_to_id(path, 0)) == 2)
        return run_binary(path, 0, argc, argv);
    sys_error("Program not found");
    return -1;
}

void dily_load(char path[], int lib_id) {
    if (! fs_does_path_exists(path) && fs_type_sector(fs_path_to_id(path, 0)) != 2) {
        sys_error("Library not found");
        return;
    }

    calloc(0x16000);
    uint8_t *binary_mem = calloc(fs_get_file_size(path) * 126);
    calloc(0x16000);
    uint32_t *file = fs_declare_read_array(path);
    fs_read_file(path, file);

    int lib_size;
    for (lib_size = 0; file[lib_size] != (uint32_t) -1; lib_size++) {
        binary_mem[lib_size] = (uint8_t) file[lib_size] & 0xFF;
    }

    free(file);

    if (lib_functions == 0) {
        lib_functions = calloc(0x1000);
        // can be realloc in the future
    }

    uint32_t *addr_list = calloc(0x1000);
    addr_list[0] = (uint32_t) lib_id;

    int addr_list_size = 1;

    for (int i = 0; i < lib_size; i++) {
        if (binary_mem[i] == 0x55 && binary_mem[i + 1] == 0x89) {
            addr_list[addr_list_size] = (uint32_t) &binary_mem[i];
            addr_list_size++;
        }
    }

    lib_functions[lib_count] = addr_list;
    lib_count++;
}

int dily_get_func(int lib_id, int func_id) {
    for (int i = 0; i < lib_count; i++) {
        if (lib_functions[i][0] != (uint32_t) lib_id) 
            continue;

        int val = (int) lib_functions[i][func_id];
        if (val == 0) {
            sys_error("Function not found");
        }
        return val;
    }

    sys_fatal("Library not found");
    return 0;
}
