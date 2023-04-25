#include <kernel/filesystem.h>
#include <kernel/snowflake.h>
#include <kernel/process.h>
#include <driver/serial.h>
#include <minilib.h>
#include <system.h>
#include <type.h>

/*************************************************************
 *       - kernel runtime memory layout and sharing -    e  *
 *                                                       l  *
 *  |             |                   | E                y  *
 *  | kernel      | allocable memory  | N                d  *
 *  | -0x100000   | - 0x200000        | D                r  *
 *  |             |                   |                  e  *
 *        ^     ^     ^     ^     ^                         *
 *        |     |     |     |     '---SCUBA----.            *
 *        |     |     |     |                  |            *
 *  |                                 | : |             |   *
 *  | physical (shared)   [STACK]     | : | virtual     |   *
 *  |                                 | : | -0xC0000000 |   *
 *  |                                 | : |             |   *
*************************************************************/

typedef struct {
    int argc;
    char **argv;
    char *path;
} comm_struct_t;

void tasked_program() {
    int pid = process_get_pid();
    int ppid = process_get_ppid(pid);

    comm_struct_t *comm = process_get_bin_mem(pid);
    int argc = comm->argc;
    char **argv = comm->argv;
    char *path = comm->path;
    free(comm);

    // setup private memory
    scuba_create_virtual(process_get_directory(pid), RUN_BIN_VBASE, RUN_BIN_VCUNT / 0x1000);

    // load binary
    fs_read_file(path, (char *) RUN_BIN_VBASE);

    // malloc a new stack
    uint32_t stack = mem_alloc(RUN_BIN_STACK, 0, 4);
    mem_set((uint8_t *) stack, 0, RUN_BIN_STACK);

    // setup stack
    asm volatile("mov %0, %%esp" : : "r" (stack + RUN_BIN_STACK));

    // call main
    int (*main)(int, char **) = (int (*)(int, char **)) RUN_BIN_VBASE;
    main(argc, argv);

    // free forgeted allocations
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

    // free the path
    free((void *) path);

    // free the argv
    for (int i = 0; i < argc; i++)
        free((void *) argv[i]);
    free((void *) argv);

    // free the stack
    free((void *) stack);

    // wake up the parent process
    int pstate = process_get_state(ppid);

    if (pstate == PROCESS_TSLPING || pstate == PROCESS_FSLPING)
        process_wakeup(ppid);

    process_exit();
}

int run_binary(char path[], int argc, char **argv) {
    // TODO: check if file is executable

    serial_debug("RUNTIME", path);
    int pid = process_create(tasked_program, 2, path);

    // duplicate path
    char *npath = malloc(str_len(path) + 1);
    str_cpy(npath, path);

    // duplicate argv
    char **nargv = malloc(sizeof(char *) * argc);

    for (int i = 0; i < argc; i++) {
        nargv[i] = malloc(str_len(argv[i]) + 1);
        str_cpy(nargv[i], argv[i]);
    }

    comm_struct_t *comm = (comm_struct_t *) mem_alloc(sizeof(comm_struct_t), 0, 6);
    comm->argc = argc;
    comm->argv = nargv;
    comm->path = npath;

    process_set_bin_mem(pid, comm);
    process_handover(pid);

    return 0;
}

int run_ifexist(char path[], int argc, char **argv) {
    if (fs_does_path_exists(path) && fs_get_sector_type(fs_path_to_id(path)) == 2)
        return run_binary(path, argc, argv);
    sys_error("Program not found");
    return -1;
}
