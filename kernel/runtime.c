#include <libc/filesystem.h>
#include <driver/serial.h>
#include <gui/gnrtx.h>
#include <libc/task.h>
#include <system.h>
#include <mem.h>


// global values for tasking
int g_return, g_argc;
char **g_argv;

void tasked_program() {
    char *binary_mem = task_get_bin_mem(task_get_current_pid());
    g_return = ((int (*)(int, char **)) binary_mem)(g_argc, g_argv);

    // fill memory with 0
    mem_set((uint8_t *) binary_mem, 0, mem_get_alloc_size((int) binary_mem));

    free(binary_mem);

    // if (task_get_next_pid() == 0) clear_screen();
    task_kill_task_switch(task_get_next_pid());
}

int run_binary(char path[], int silence, int argc, char **argv) {
    // TODO: check if file is executable
    // TODO: check if there is enough memory

    serial_debug("RUNTIME", path);
    (void) silence;

    int pid = task_create(tasked_program, path);

    char *binary_mem = calloc(fs_get_file_size(path) * 126);
    uint32_t *file = fs_declare_read_array(path);
    fs_read_file(path, file);

    for (int i = 0; file[i] != (uint32_t) -1 ; i++)
        binary_mem[i] = (char) file[i];

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
