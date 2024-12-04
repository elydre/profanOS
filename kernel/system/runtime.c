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

int binary_exec(uint32_t sid, int argc, char **argv, char **envp) {
    int pid = process_get_pid();
    uint32_t *phys;

    if (IS_SID_NULL(sid) || !fu_is_file(fs_get_main(), sid)) {
        sys_error("[binary_exec] File not found");
        return process_kill(pid, 1);
    }

    comm_struct_t *comm = process_get_comm(pid);

    // free the argv
    if (comm->argv) {
        for (int i = 0; comm->argv[i] != NULL; i++)
            free(comm->argv[i]);
        free(comm->argv);
    }
    free(comm->envp);

    comm->argv = argv;
    comm->envp = envp;
    process_set_name(pid, argv[0] ? argv[0] : "unknown");

    uint32_t fsize = fs_cnt_get_size(fs_get_main(), sid);

    scuba_dir_t *old_dir = process_get_dir(pid);
    scuba_dir_t *new_dir = scuba_dir_inited(old_dir, pid);

    // create stack
    phys = scuba_create_virtual(new_dir, (void *) PROC_ESP_ADDR, PROC_ESP_SIZE / 0x1000);
    mem_copy(phys, (void *) PROC_ESP_ADDR, PROC_ESP_SIZE);

    process_switch_directory(pid, new_dir, 0);

    // switch to new directory
    scuba_switch(new_dir);

    scuba_dir_destroy(old_dir);

    // load binary
    fs_cnt_read(fs_get_main(), sid, (void *) RUN_BIN_VBASE + RUN_BIN_OFFSET, 0, fsize);
    mem_set((void *) RUN_BIN_VBASE + RUN_BIN_OFFSET + fsize, 0, RUN_BIN_VCUNT - RUN_BIN_OFFSET - fsize);

    // call main
    int (*main)(int, char **, char **) = (int (*)(int, char **, char **)) RUN_BIN_VBASE + RUN_BIN_OFFSET;

    sys_exit_kernel(0);
    int ret = main(argc, argv, envp) & 0xFF;
    sys_entry_kernel(0);

    return process_kill(process_get_pid(), ret);
}

int run_ifexist(char *file, int sleep, char **argv, int *pid_ptr) {
    uint32_t sid = fu_path_to_sid(fs_get_main(), SID_ROOT, file);

    if (IS_SID_NULL(sid) || !fu_is_file(fs_get_main(), sid)) {
        sys_warning("[run_ifexist] File not found: %s", file);
        return -1;
    }

    int argc = 0;
    while (argv && argv[argc] != NULL)
        argc++;

    char **nargv = mem_alloc((argc + 1) * sizeof(char *), 0, 6);
    mem_set((void *) nargv, 0, (argc + 1) * sizeof(char *));

    for (int i = 0; i < argc; i++) {
        nargv[i] = mem_alloc(str_len(argv[i]) + 1, 0, 6);
        str_cpy(nargv[i], argv[i]);
    }

    int pid = process_create(binary_exec, 0, 4, (uint32_t []) {sid, 0, (uint32_t) nargv, 0});

    if (pid_ptr != NULL)
        *pid_ptr = pid;

    if (pid == -1)
        return -1;

    if (sleep == 2)
        return 0;

    uint8_t ret;

    process_wakeup(pid, sleep);
    process_wait(pid, &ret, 0);

    return ret;
}
