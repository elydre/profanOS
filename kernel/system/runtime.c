/*****************************************************************************\
|   === runtime.c : 2024 ===                                                  |
|                                                                             |
|    Kernel binary execution functions                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/snowflake.h>
#include <kernel/butterfly.h>
#include <kernel/process.h>
#include <minilib.h>
#include <system.h>

int force_exit_pid(int pid, int ret_code, int warn_leaks) {
    int ppid, pstate, leaks;

    if (pid == 0 || pid == 1) {
        return 1;
    }

    comm_struct_t *comm = process_get_comm(pid);
    if (comm != NULL) {
        // free the argv
        for (int i = 0; comm->argv[i] != NULL; i++)
            free(comm->argv[i]);
        free(comm->argv);
        free(comm->envp);

        // free comm struct
        free(comm);
    }

    if (warn_leaks && (leaks = mem_get_info(7, pid)) > 0) {
        sys_warning("Memory leak of %d alloc%s (pid %d, %d bytes)",
                leaks,
                leaks == 1 ? "" : "s",
                pid,
                mem_get_info(8, pid)
        );
    }
    mem_free_all(pid);

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

int binary_exec(uint32_t sid, int argc, char **argv, char **envp) {
    int pid = process_get_pid();

    if (IS_SID_NULL(sid) || !fu_is_file(fs_get_main(), sid)) {
        sys_error("[binary_exec] File not found");
        return force_exit_pid(pid, 1, 0);
    }

    comm_struct_t *comm = process_get_comm(pid);

    if (comm != NULL) {
        // free the argv
        for (int i = 0; comm->argv[i] != NULL; i++)
            free(comm->argv[i]);
        free(comm->argv);
        free(comm->envp);
    } else {
        comm = (void *) mem_alloc(sizeof(comm_struct_t), 0, 6);
    }

    comm->argv = argv;
    comm->envp = envp;
    process_set_comm(pid, comm);

    uint32_t fsize = fs_cnt_get_size(fs_get_main(), sid);
    uint32_t real_fsize = fsize;

    scuba_directory_t *dir = scuba_directory_inited();

    // create program memory
    scuba_create_virtual(dir, (void *) RUN_BIN_VBASE, RUN_BIN_VCUNT / 0x1000);

    // create stack
    uint32_t *phys_stack = scuba_create_virtual(dir, (void *) PROC_ESP_ADDR, PROC_ESP_SIZE / 0x1000);

    mem_copy(phys_stack, (void *) PROC_ESP_ADDR, PROC_ESP_SIZE);

    process_switch_directory(pid, dir, 0);

    // switch to new directory
    asm volatile("mov %0, %%cr3":: "r"(dir));

    // load binary
    fs_cnt_read(fs_get_main(), sid, (void *) RUN_BIN_VBASE, 0, real_fsize);

    // call main

    int (*main)(int, char **, char **) = (int (*)(int, char **, char **)) RUN_BIN_VBASE;

    sys_exit_kernel(0);
    int ret = main(argc, argv, envp) & 0xFF;
    sys_entry_kernel(0);

    return force_exit_pid(process_get_pid(), ret, 1);
}

int run_ifexist(char *file, int sleep, char **argv, int *pid_ptr) {
    uint32_t sid = fu_path_to_sid(fs_get_main(), ROOT_SID, file);

    if (IS_SID_NULL(sid) || !fu_is_file(fs_get_main(), sid)) {
        sys_warning("[run_ifexist] File not found: %s", file);
        return -1;
    }

    int argc = 0;
    while (argv && argv[argc] != NULL)
        argc++;

    char **nargv = (char **) mem_alloc((argc + 1) * sizeof(char *), 0, 6);
    mem_set((void *) nargv, 0, (argc + 1) * sizeof(char *));

    for (int i = 0; i < argc; i++) {
        nargv[i] = (char *) mem_alloc(str_len(argv[i]) + 1, 0, 6);
        str_cpy(nargv[i], argv[i]);
    }

    int pid = process_create(binary_exec, 0, file, 4, (uint32_t []) {sid, 0, (uint32_t) nargv, 0});

    if (pid_ptr != NULL)
        *pid_ptr = pid;

    if (pid == -1)
        return -1;

    if (sleep == 2)
        return 0;

    if (sleep)
        process_handover(pid);
    else
        process_wakeup(pid);

    return process_get_info(pid, PROCESS_INFO_EXIT_CODE);
}
