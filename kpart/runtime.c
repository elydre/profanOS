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
    uint32_t vbase;
    uint32_t vcunt;
    uint32_t stack_size;
} comm_struct_t;

void tasked_program() {
    int pid = process_get_pid();
    int ppid = process_get_ppid(pid);

    comm_struct_t *comm = process_get_bin_mem(pid);
    uint32_t stack_size = comm->stack_size;

    int vsize = comm->vcunt;
    int fsize = fs_get_file_size(comm->path);
    fsize = fsize + (0x1000 - (fsize % 0x1000));

    if (vsize < fsize * 2)
        vsize = fsize * 2;

    // setup private memory
    scuba_create_virtual(process_get_directory(pid), comm->vbase, vsize / 0x1000);

    // load binary
    fs_read_file(comm->path, (char *) comm->vbase);

    // malloc a new stack
    uint32_t stack = mem_alloc(stack_size, 0, 4);
    mem_set((uint8_t *) stack, 0, stack_size);

    // setup stack
    asm volatile("mov %0, %%esp" :: "r" (stack + stack_size));

    // call main
    int (*main)(int, char **) = (int (*)(int, char **)) comm->vbase;
    main(comm->argc, comm->argv);


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
    free((void *) comm->path);

    // free the argv
    for (int i = 0; i < comm->argc; i++)
        free((void *) comm->argv[i]);
    free((void *) comm->argv);

    // free comm struct
    free(comm);

    // free the stack
    free((void *) stack);

    // wake up the parent process
    int pstate = process_get_state(ppid);

    if (pstate == PROCESS_TSLPING || pstate == PROCESS_FSLPING)
        process_wakeup(ppid);

    process_exit();
}

int run_binary(char path[], int argc, char **argv, uint32_t vbase, uint32_t vcunt, uint32_t stack_size) {
    // TODO: check if file is executable

    serial_debug("RUNTIME", path);
    int pid = process_create(tasked_program, 2, path);

    // duplicate path
    char *npath = malloc(str_len(path) + 1);
    str_cpy(npath, path);

    // duplicate argv
    char **nargv = calloc(sizeof(char *) * (argc + 1));

    for (int i = 0; i < argc; i++) {
        nargv[i] = malloc(str_len(argv[i]) + 1);
        str_cpy(nargv[i], argv[i]);
    }

    comm_struct_t *comm = (comm_struct_t *) mem_alloc(sizeof(comm_struct_t), 0, 6);
    comm->argc = argc;
    comm->argv = nargv;
    comm->path = npath;

    comm->vbase = vbase;
    comm->vcunt = vcunt;
    comm->stack_size = stack_size;

    process_set_bin_mem(pid, comm);
    process_handover(pid);

    return 0;
}

int run_ifexist_full(char path[], int argc, char **argv, uint32_t vbase, uint32_t vcunt, uint32_t stack) {
    vbase = vbase ? vbase : RUN_BIN_VBASE;
    vcunt = vcunt ? vcunt : RUN_BIN_VCUNT;
    stack = stack ? stack : RUN_BIN_STACK;

    if (fs_does_path_exists(path) && fs_get_sector_type(fs_path_to_id(path)) == 2)
        return run_binary(path, argc, argv, vbase, vcunt, stack);

    sys_error("Program not found");
    return -1;
}
