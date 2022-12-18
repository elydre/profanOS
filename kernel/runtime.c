#include <kernel/filesystem.h>
#include <kernel/snowflake.h>
#include <driver/serial.h>
#include <kernel/task.h>
#include <minilib.h>
#include <system.h>
#include <type.h>

/******************************
 * binarymem is a pointer to *
 * the memory of the program *
 * with the following layout *
 * |  <---STACK-|--BINARY--| *
******************************/

// global values for tasking
int g_return, g_argc;
char **g_argv;

uint32_t **lib_functions = 0;
int lib_count = 0;

void tasked_program() {
    int pid = task_get_current_pid();
    uint8_t *binary_mem = task_get_bin_mem(pid);
    g_return = ((int (*)(int, char **)) binary_mem + RUN_STACK_BIN)(g_argc, g_argv);

    free(binary_mem);

    int not_free_mem = mem_get_info(7, pid);

    if (!not_free_mem) {
        task_kill_task_switch(task_get_next_pid());
        return;
    }

    sys_warning("Memory leak detected");

    kprintf("$6[auto free] %d alloc will be auto freed (total: %d bytes, pid: %d)\n",
            not_free_mem,
            mem_get_info(8, pid),
            pid
    );

    mem_free_all(pid);

    task_kill_task_switch(task_get_next_pid());
}

int run_binary(char path[], int silence, int argc, char **argv) {
    // TODO: check if file is executable
    // TODO: check if there is enough memory

    serial_debug("RUNTIME", path);
    (void) silence;

    int pid = task_create(tasked_program, path);

    int size = fs_get_file_size(path) + RUN_STACK_BIN;
    uint8_t *binary_mem = (uint8_t *) mem_alloc(size, 4); // 4 = runtime
    uint8_t *file = binary_mem + RUN_STACK_BIN;

    fs_read_file(path, (char *) file);

    g_argc = argc;
    g_argv = argv;
    g_return = 0;

    task_set_bin_mem(pid, binary_mem);

    task_switch(pid);

    // TODO: memory leak detection

    return g_return;
}

int run_ifexist(char path[], int argc, char **argv) {
    if (fs_does_path_exists(path) && fs_get_sector_type(fs_path_to_id(path)) == 2)
        return run_binary(path, 0, argc, argv);
    sys_error("Program not found");
    return -1;
}

void dily_load(char path[], int lib_id) {
    if ((!fs_does_path_exists(path)) || fs_get_sector_type(fs_path_to_id(path)) != 2) {
        sys_error("Library not found");
        return;
    }

    if (lib_functions == 0) {
        lib_functions = calloc(0x1000);
        // can be realloc in the future
    }

    int lib_size = fs_get_file_size(path) + RUN_STACK_LIB;
    uint8_t *binary_mem = (uint8_t *) mem_alloc(lib_size, 5); // 5 = library
    uint8_t *file = binary_mem + RUN_STACK_LIB;

    fs_read_file(path, (char *) file);

    uint32_t *addr_list = calloc(0x1000);
    addr_list[0] = (uint32_t) lib_id;

    int addr_list_size = 1;

    for (int i = 0; i < lib_size; i++) {
        if (file[i] == 0x55 && file[i + 1] == 0x89) {
            addr_list[addr_list_size] = (uint32_t) &file[i];
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

void dily_unload(int lib_id) {
    for (int i = 0; i < lib_count; i++) {
        if (lib_functions[i][0] != (uint32_t) lib_id) 
            continue;

        free(lib_functions[i]);
        lib_functions[i] = 0;
        return;
    }

    sys_error("Library not found");
}
