#include <kernel/filesystem.h>
#include <kernel/snowflake.h>
#include <driver/serial.h>
#include <kernel/process.h>
#include <minilib.h>
#include <system.h>
#include <type.h>

/*******************************************
 * binary_mem is pointer to the memory of *
 * the program with the following layout. *
 * |  <--Lstack--|--BINARY--|-Rstack->  | *
*******************************************/

// global values for tasking
char **g_argv;
int g_argc;

void tasked_program() {
    int pid = process_get_running_pid();
    int ppid = process_get_ppid(pid);

    uint8_t *binary_mem = process_get_bin_mem(pid);
    ((int (*)(int, char **)) binary_mem + RUN_BIN_STACK_L)(g_argc, g_argv);

    free(binary_mem);

    int not_free_mem = mem_get_info(7, pid);

    if (not_free_mem) {
        sys_warning("Memory leak detected");

        kprintf("[auto free] %d alloc will be auto freed (total: %d bytes, pid: %d)\n",
                not_free_mem,
                mem_get_info(8, pid),
                pid
        );

        mem_free_all(pid);
    }

    process_wakeup(ppid);
    process_exit();
}

int run_binary(char path[], int argc, char **argv) {
    UNUSED(path);
    UNUSED(argc);
    UNUSED(argv);
    // TODO: check if file is executable

    serial_debug("RUNTIME", path);
    int pid = process_create(tasked_program, 2, path);

    int size = fs_get_file_size(path) + RUN_BIN_STACK_L + RUN_BIN_STACK_R;
    uint8_t *binary_mem = (uint8_t *) mem_alloc(size, 4); // 4 = runtime
    uint8_t *file = binary_mem + RUN_BIN_STACK_L;

    if (!binary_mem) {
        sys_error("Not enough memory");
        return -1;
    }

    fs_read_file(path, (char *) file);

    g_argc = argc;
    g_argv = argv;

    process_set_bin_mem(pid, binary_mem);
    process_handover(pid);

    return 0;
}

int run_ifexist(char path[], int argc, char **argv) {
    if (fs_does_path_exists(path) && fs_get_sector_type(fs_path_to_id(path)) == 2)
        return run_binary(path, argc, argv);
    sys_error("Program not found");
    return -1;
}
