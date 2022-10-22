#include <driver/serial.h>
#include <filesystem.h>
#include <system.h>
#include <task.h>
#include <mem.h>


// global values for tasking
char **g_argv;
int g_argc;

void tasked_program() {
    char * binary_mem = task_get_bin_mem(task_get_current_pid());
    int (*start_program)() = (int (*)())(binary_mem);
    start_program(g_argc, g_argv);

    free(binary_mem);

    task_kill_yield(task_get_next_pid());
}

int run_binary(char path[], int silence, int argc, char **argv) {
    serial_debug("RUNTIME", path);
    (void) silence;

    char * binary_mem = calloc(fs_get_file_size(path)*126);
    uint32_t * file = fs_declare_read_array(path);
    fs_read_file(path, file);

    for (int i = 0; file[i] != (uint32_t) -1 ; i++)
        binary_mem[i] = (char) file[i];

    free(file);

    g_argc = argc;
    g_argv = argv;

    int pid = task_create(tasked_program, path);
    task_set_bin_mem(pid, binary_mem);   

    yield(pid);

    // TODO: memory leak detection

    return 0;
}

int run_ifexist(char path[], int argc, char **argv) {
    if (fs_does_path_exists(path) && fs_type_sector(fs_path_to_id(path, 0)) == 2)
        return run_binary(path, 0, argc, argv);
    sys_error("Program not found");
    return -1;
}
