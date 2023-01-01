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

void tasked_program() {
    int pid = task_get_current_pid();
    uint8_t *binary_mem = task_get_bin_mem(pid);
    uint8_t *entry = binary_mem + RUN_STACK_BIN;
    while (entry[0] != 0x55 && entry[1] != 0x89) entry += 2;
    g_return = ((int (*)(int, char **)) entry)(g_argc, g_argv);

    free(binary_mem);

    int not_free_mem = mem_get_info(7, pid);

    if (!not_free_mem) {
        task_kill_task_switch(task_get_next_pid());
        return;
    }

    sys_warning("Memory leak detected");

    kprintf("[auto free] %d alloc will be auto freed (total: %d bytes, pid: %d)\n",
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
