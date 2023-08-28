#include <kernel/snowflake.h>
#include <kernel/butterfly.h>
#include <kernel/process.h>
#include <drivers/serial.h>
#include <minilib.h>
#include <system.h>
#include <ktype.h>

/************************************************************
 *     - kernel runtime memory layout and sharing -     e  *
 *                                                      l  *
 *  |             |                   | E               y  *
 *  | kernel      | allocable memory  | N               d  *
 *  | -0x100000   | - 0x200000        | D               r  *
 *  |             |                   |                 e  *
 *        ^     ^     ^     ^     ^                        *
 *        |     |     |     |     '---SCUBA----.           *
 *        |     |     |     |                  |           *
 *  |                                 | : |             |  *
 *  | physical (shared)   [STACK]     | : | virtual     |  *
 *  |                                 | : | -0xC0000000 |  *
 *  |                                 | : |             |  *
************************************************************/

typedef struct {
    int pid;
    int ret;
} direct_return_t;

direct_return_t last_return;

int force_exit_pid(int pid, int ret_code) {
    // clean memory
    mem_free_all(pid);

    comm_struct_t *comm = process_get_comm(pid);

    if (comm != NULL) {
        // free the argv
        for (int i = 0; i < comm->argc; i++)
            free((void *) comm->argv[i]);
        free((void *) comm->argv);

        // free the stack
        free((void *) comm->stack);

        // free comm struct
        free(comm);
    }

    // set return value
    last_return.pid = pid;
    last_return.ret = ret_code;

    // wake up the parent process
    int pstate = process_get_state(process_get_ppid(pid));

    if (pstate == PROCESS_TSLPING || pstate == PROCESS_FSLPING)
        process_wakeup(process_get_ppid(pid));

    return process_kill(pid);
}

void tasked_program() {
    int pid = process_get_pid();
    int ppid = process_get_ppid(pid);

    comm_struct_t *comm = process_get_comm(pid);
    uint32_t stack_size = comm->stack_size;

    int vsize = comm->vcunt;
    int fsize = fs_cnt_get_size(fs_get_main(), comm->file);
    int real_fsize = fsize;

    // setup private memory
    fsize = fsize + (0x1000 - (fsize % 0x1000));

    if (vsize < fsize * 2)
        vsize = fsize * 2;

    scuba_create_virtual(process_get_directory(pid), comm->vbase, vsize / 0x1000);

    // load binary
    fs_cnt_read(fs_get_main(), comm->file, (uint8_t *) comm->vbase, 0, real_fsize);

    // malloc a new stack
    comm->stack = mem_alloc(stack_size, 0, 4);
    mem_set((uint8_t *) comm->stack, 0, stack_size);

    // setup stack
    uint32_t old_esp = 0;
    asm volatile("mov %%esp, %0" : "=r" (old_esp));
    asm volatile("mov %0, %%esp" :: "r" (comm->stack + stack_size - 0x80));

    // call main
    int (*main)(int, char **) = (int (*)(int, char **)) comm->vbase;
    int ret = main(comm->argc, comm->argv) & 0xFF;

    // restore stack
    asm volatile("mov %0, %%esp" :: "r" (old_esp));

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

    // free the comm struct
    for (int i = 0; i < comm->argc; i++)
        free((void *) comm->argv[i]);

    free((void *) comm->stack);
    free((void *) comm->argv);
    free(comm);

    // set return value
    last_return.pid = pid;
    last_return.ret = ret;

    // wake up the parent process
    int pstate = process_get_state(ppid);

    if (pstate == PROCESS_TSLPING || pstate == PROCESS_FSLPING)
        process_wakeup(ppid);

    process_kill(pid);
}

int run_binary(runtime_args_t args, int *pid_ptr) {
    serial_debug("RUNTIME", args.path);

    int pid = process_create(tasked_program, 2, args.path);

    if (pid == -1)
        return -1;

    if (pid == 2)
        last_return.pid = -1;

    if (pid_ptr != NULL)
        *pid_ptr = pid;

    // duplicate argv
    int size = sizeof(char *) * (args.argc + 1);
    char **nargv = (char **) mem_alloc(size, 0, 6);
    mem_set((void *) nargv, 0, size);

    for (int i = 0; i < args.argc; i++) {
        nargv[i] = (char *) mem_alloc(str_len(args.argv[i]) + 1, 0, 6);
        str_cpy(nargv[i], args.argv[i]);
    }

    comm_struct_t *comm = (comm_struct_t *) mem_alloc(sizeof(comm_struct_t), 0, 6);
    comm->argc = args.argc;
    comm->argv = nargv;
    comm->file = args.sid;

    comm->vbase = args.vbase;
    comm->vcunt = args.vcunt;
    comm->stack_size = args.stack;

    process_set_comm(pid, comm);

    if (args.sleep)
        process_handover(pid);
    else
        process_wakeup(pid);

    return (last_return.pid == pid) ? last_return.ret : 0;
}

int run_ifexist_full(runtime_args_t args, int *pid) {
    /*vbase = vbase ? vbase : RUN_BIN_VBASE;
    vcunt = vcunt ? vcunt : RUN_BIN_VCUNT;
    stack = stack ? stack : RUN_BIN_STACK;

    sid_t file = fu_path_to_sid(fs_get_main(), ROOT_SID, path);
    */

    if (args.path == NULL && IS_NULL_SID(args.sid)) {
        sys_error("No path or sid provided");
        return -1;
    }

    args.path = args.path == NULL ? "serenity" : args.path;
    args.sid = IS_NULL_SID(args.sid) ? fu_path_to_sid(fs_get_main(), ROOT_SID, args.path) : args.sid;

    args.vbase = args.vbase ? args.vbase : RUN_BIN_VBASE;
    args.vcunt = args.vcunt ? args.vcunt : RUN_BIN_VCUNT;
    args.stack = args.stack ? args.stack : RUN_BIN_STACK;

    if (!IS_NULL_SID(args.sid) && fu_is_file(fs_get_main(), args.sid)) {
        return run_binary(args, pid);
    }

    sys_error("Program not found");
    return -1;
}
