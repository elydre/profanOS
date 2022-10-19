#include <filesystem.h>
#include <system.h>
#include <system.h>
#include <task.h>
#include <mem.h>

#include <iolib.h>

// global values for tasking
int g_argc, g_ret, g_state;
char **g_argv;
char * g_mem;

void tasked_program() {
    int (*start_program)() = (int (*)())(g_mem);
    g_ret = start_program(g_argc, g_argv);
    g_state = 0;
    task_kill_yield(0);
}

int sys_run_binary(char path[], int silence, int argc, char **argv) {
    uint32_t usbl_mem = (uint32_t) mem_get_usable() - mem_get_usage();
    if (usbl_mem < 16 || usbl_mem < fs_get_file_size(path) + 16)
        return sys_error("Not enough memory to run this program");

    char * binary_mem = calloc(fs_get_file_size(path)*126);
    uint32_t * file = fs_declare_read_array(path);

    fs_read_file(path, file);

    for (int i = 0; file[i] != (uint32_t) -1 ; i++)
        binary_mem[i] = (char) file[i];
    
    free(file);

    int old_active_alloc = mem_get_alloc_count() - mem_get_free_count();

    g_argc = argc;
    g_argv = argv;
    g_mem = binary_mem;
    g_state = 0;
    
    int pid = task_create(tasked_program);
    yield(pid);

    if (g_state == 0) {
        fskprint("exit by yield\n");
        return g_ret;
    }
    
    if (!silence) {
        if (old_active_alloc < mem_get_alloc_count() - mem_get_free_count())
            sys_warning("Memory leak detected");
        else if (old_active_alloc > mem_get_alloc_count() - mem_get_free_count())
            sys_warning("Memory void detected");
    }
    
    free(binary_mem);
    return g_ret;
}

int sys_run_ifexist(char path[], int argc, char **argv) {
    if (fs_does_path_exists(path) && fs_type_sector(fs_path_to_id(path, 0)) == 2)
        return sys_run_binary(path, 0, argc, argv);
    sys_error("Program not found");
    return -1;
}
