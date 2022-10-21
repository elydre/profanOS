#include <filesystem.h>
#include <system.h>
#include <system.h>
#include <task.h>
#include <mem.h>

#include <iolib.h>

// global values for tasking
int g_argc, g_status;
char * binary_mem;
char **g_argv;

void tasked_program() {
    int (*start_program)() = (int (*)())(binary_mem);
    start_program(g_argc, g_argv);

    g_status = 0;

    free(binary_mem);

    fskprint("Program exited, yielding to kernel\n");

    task_kill_yield(0);
}

int sys_run_binary(char path[], int silence, int argc, char **argv) {
    (void) silence;

    binary_mem = calloc(fs_get_file_size(path)*126);
    uint32_t * file = fs_declare_read_array(path);

    fs_read_file(path, file);

    for (int i = 0; file[i] != (uint32_t) -1 ; i++)
        binary_mem[i] = (char) file[i];
    
    free(file);

    g_argc = argc;
    g_argv = argv;

    g_status = 1;
    
    int pid = task_create(tasked_program, path);
    yield(pid);

    if (g_status ) {
        fskprint("exit by yield\n");
    } else {
        fskprint("exit without yield\n");
    }
    
    // TODO: memory leak detection
    
    return 0;
}

int sys_run_ifexist(char path[], int argc, char **argv) {
    if (fs_does_path_exists(path) && fs_type_sector(fs_path_to_id(path, 0)) == 2)
        return sys_run_binary(path, 0, argc, argv);
    sys_error("Program not found");
    return -1;
}
