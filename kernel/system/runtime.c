#include <kernel/snowflake.h>
#include <kernel/butterfly.h>
#include <kernel/process.h>
#include <minilib.h>
#include <system.h>

/************************************************************
 *     - kernel runtime memory layout and sharing -     e  *
 *                                                      l  *
 *  |             |                   | E               y  *
 *  | kernel      | allocable memory  | N               d  *
 *  | `0x100000   | `0x200000         | D               r  *
 *  |             |                   |                 e  *
 *        ^     ^     ^     ^     ^                        *
 *        |     |     |     |     '---SCUBA----.           *
 *        |     |     |     |                  |           *
 *  |                                 | : |             |  *
 *  | physical (shared)   [STACK]     | : | virtual     |  *
 *  |                                 | : | `0xC0000000 |  *
 *  |                                 | : |             |  *
************************************************************/

int force_exit_pid(int pid, int ret_code) {
    int ppid, pstate;

    if (pid == 0 || pid == 1) {
        return 1;
    }

    int not_free_mem = mem_get_info(7, pid);

    if (not_free_mem) {
        sys_warning("Memory leak of %d alloc%s (pid %d, %d bytes)",
                not_free_mem,
                not_free_mem == 1 ? "" : "s",
                pid,
                mem_get_info(8, pid)
        );

        mem_free_all(pid);
    }

    comm_struct_t *comm = process_get_comm(pid);

    if (comm != NULL) {
        // free the argv
        for (int i = 0; comm->argv[i] != NULL; i++)
            free((void *) comm->argv[i]);
        free((void *) comm->argv);

        // free the stack
        // free((void *) comm->stack);

        // free comm struct
        free(comm);
    }

    // set return value
    process_set_return(pid, ret_code);

    // wake up the parent process
    ppid = process_get_ppid(pid);

    if (ppid != -1) {
        pstate = process_get_state(ppid);

        if (pstate == PROCESS_TSLPING || pstate == PROCESS_FSLPING) {
            process_wakeup(ppid);
        }
    }

    return process_kill(pid);
}

void binary_exec(sid_t sid, uint32_t vcunt, char **argv) {
    kprintf_serial("binary_exec s%dd%d, vcunt %x, argv %x\n", sid.device, sid.sector, vcunt, argv);

    int pid = process_get_pid();

    comm_struct_t *comm = (void *) mem_alloc(sizeof(comm_struct_t), 0, 6);
    comm->argv = argv;
    process_set_comm(pid, comm);

    uint32_t fsize = fs_cnt_get_size(fs_get_main(), sid);
    uint32_t real_fsize = fsize;

    // setup private memory
    fsize = fsize + (0x1000 - (fsize % 0x1000));

    if (vcunt < fsize * 2)
        vcunt = fsize * 2;

    scuba_directory_t *dir = scuba_directory_inited();
    scuba_create_virtual(dir, RUN_BIN_VBASE, vcunt / 0x1000);
    process_switch_directory(pid, dir);

    // get argc
    int argc = 0;
    while (argv && argv[argc] != NULL)
        argc++;

    // load binary
    fs_cnt_read(fs_get_main(), sid, (void *) RUN_BIN_VBASE, 0, real_fsize);

    // TODO: reset the stack

    // call main
    int (*main)(int, char **) = (int (*)(int, char **)) RUN_BIN_VBASE;
    force_exit_pid(process_get_pid(), main(argc, argv) & 0xFF);
}

int run_ifexist_full(runtime_args_t args, int *pid_ptr) {
    if (args.path == NULL && IS_NULL_SID(args.sid)) {
        sys_warning("[run_ifexist] No path or sid given");
        return -1;
    }

    args.sid = IS_NULL_SID(args.sid) ? fu_path_to_sid(fs_get_main(), ROOT_SID, args.path) : args.sid;

    args.vcunt = args.vcunt ? args.vcunt : RUN_BIN_VCUNT;

    if (IS_NULL_SID(args.sid) || !fu_is_file(fs_get_main(), args.sid)) {
        if (args.path)
            sys_warning("[run_ifexist] File not found: %s", args.path);
        else
            sys_warning("[run_ifexist] Sector d%ds%d is not a file", args.sid.device, args.sid.sector);
        return -1;
    }

    // duplicate argv
    int size = sizeof(char *) * (args.argc + 1);
    char **nargv = (char **) mem_alloc(size, 0, 6);
    mem_set((void *) nargv, 0, size);

    for (int i = 0; i < args.argc; i++) {
        nargv[i] = (char *) mem_alloc(str_len(args.argv[i]) + 1, 0, 6);
        str_cpy(nargv[i], args.argv[i]);
    }

    int pid = process_create(binary_exec, 1, args.path ? args.path : "unknown file", 4, args.sid, args.vcunt, nargv);

    if (pid_ptr != NULL)
        *pid_ptr = pid;
    if (pid == -1)
        return -1;

    if (args.sleep == 2)
        return 0;

    if (args.sleep)
        process_handover(pid);
    else
        process_wakeup(pid);

    return process_get_info(pid, PROCESS_INFO_EXIT_CODE);
}
