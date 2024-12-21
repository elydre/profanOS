/*****************************************************************************\
|   === rosemary.c : 2024 ===                                                 |
|                                                                             |
|    Initialisation program for profanOS userspace.                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define _SYSCALL_CREATE_FUNCS

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/libmmq.h>
#include <profan/panda.h>
#include <profan/clip.h>
#include <profan.h>

#include <fcntl.h> // open flags

#define LOADER_NAME "rosemary"

#define SHELL_PATH "/bin/fatpath/olivine.elf"
#define SHELL_NAME "olivine"

#define START_USAGE_GRAPH 1

typedef struct {
    int id;
    char *path;
} mod_t;

mod_t mods_at_boot[] = {
    {1001, "/lib/mod/libmmq.pok"},
    {1002, "/lib/mod/filesys.pok"},
    {1003, "/lib/mod/devio.pok"},
    {1004, "/lib/mod/fmopen.pok"},
    {1005, "/lib/mod/profan.pok"},
    {1006, "/lib/mod/panda.pok"},
    {1007, "/lib/mod/dlgext.pok"},
};

int local_strlen(char *str) {
    int i = 0;
    while (str[i])
        i++;
    return i;
}

char *get_name(char *path) {
    int i = local_strlen(path) - 1;
    while (i >= 0 && path[i] != '/') {
        i--;
    }
    return path + i + 1;
}

int print_load_status(int i) {
    mod_t *mod = &mods_at_boot[i];
    int error_code = syscall_pok_load(mod->path, mod->id);

    if (error_code > 0)
        return 0;

    syscall_kprint("["LOADER_NAME"] FATAL: failed to load ");
    syscall_kprint(get_name(mod->path));
    syscall_kprint(": ");

    switch (error_code) {
        case -1:
            syscall_kprint("file not found\n");
            break;
        case -2:
            syscall_kprint("invalid module id range\n");
            break;
        case -3:
            syscall_kprint("file is not a dynamic ELF\n");
            break;
        case -4:
            syscall_kprint("ELF relocation failed\n");
            break;
        case -5:
            syscall_kprint("failed to read functions\n");
            break;
        case -6:
            syscall_kprint("init function exited with error\n");
            break;
        default:
            syscall_kprint("unknown error\n");
            break;
    }

    return 1;
}

void rainbow_print(char *message) {
    char rainbow_colors[] = {'2', '6', '4', '5', '1', '3'};
    char tmp[] = "\e[9XmX";

    for (int i = 0; message[i]; i++) {
        tmp[3] = rainbow_colors[i % 6];
        tmp[5] = message[i];
        fd_putstr(1, tmp);
    }
}

void welcome_print(void) {
    rainbow_print("Welcome to profanOS!");

    fd_putstr(1, "\n\e[35mKernel: \e[95m");
    fd_putstr(1, syscall_sys_kinfo());
    fd_putstr(1, "\e[0m\n\n");
}

char wait_key(void) {
    while (syscall_sc_get());

    char key_char = 0;

    do {
        syscall_process_sleep(syscall_process_pid(), 50);
        key_char = profan_kb_get_char(syscall_sc_get(), 0);
    } while (key_char != 'g' && key_char != 'h' && key_char != 'j');

    return key_char;
}

char **envp;

void set_env(char *line) {
    if (envp == NULL) {
        envp = kmalloc(2 * sizeof(char *));
        envp[0] = line;
        envp[1] = NULL;
    } else {
        int i;
        for (i = 0; envp[i]; i++);
        envp = krealloc(envp, (i + 2) * sizeof(char *));
        envp[i] = line;
        envp[i + 1] = NULL;
    }
}

int main(void) {
    char key_char, use_panda = 0;
    int total, usage_pid;

    envp = NULL;

    total = (int) (sizeof(mods_at_boot) / sizeof(mod_t));

    for (int i = 0; i < total; i++) {
        if (!print_load_status(i))
            continue;
        syscall_kprint("["LOADER_NAME"] Module loading failed, exiting\n");
        return 1;
    }

    fd_printf(1, "Successfully loaded %d modules\n\n", total);

    if (syscall_vesa_state()) {
        panda_set_start(syscall_get_cursor());
        use_panda = 1;
        if (fm_reopen(0, "/dev/panda", O_RDONLY)  < 0 ||
            fm_reopen(1, "/dev/panda", O_WRONLY)  < 0 ||
            fm_reopen(2, "/dev/pander", O_WRONLY) < 0
        ) syscall_kprint("["LOADER_NAME"] Failed to redirect to panda\n");
        set_env("TERM=/dev/panda");
        syscall_sys_set_reporter(userspace_reporter);
        if (START_USAGE_GRAPH)
            run_ifexist_full((runtime_args_t){"/bin/tools/usage.elf", 1, (char *[]){"usage"}, NULL, 0}, &usage_pid);
    } else {
        syscall_kprint("["LOADER_NAME"] Using kernel output for stdout\n");
        if (fm_reopen(0, "/dev/kterm", O_RDONLY)  < 0 ||
            fm_reopen(1, "/dev/kterm", O_WRONLY)  < 0 ||
            fm_reopen(2, "/dev/kterm", O_WRONLY) < 0
        ) syscall_kprint("["LOADER_NAME"] Failed to redirect to kterm\n");
        set_env("TERM=/dev/kterm");
    }

    welcome_print();

    set_env("PATH=/bin/cmd:/bin/fatpath");
    set_env("DEFRUN=/bin/fatpath/tcc.elf -run");
    set_env("HOME=/");
    set_env("PWD=/");

    do {
        run_ifexist_full((runtime_args_t){SHELL_PATH, 1, (char *[]){SHELL_NAME}, envp, 1}, NULL);

        fd_putstr(1, "\n["LOADER_NAME"] "SHELL_NAME" exited,\nAction keys:\n"
            " g - start "SHELL_NAME" again\n"
            " h - unload all modules and exit\n"
            " j - reboot profanOS\n"
        );

        if ((key_char = wait_key()) == 'j') {
            syscall_sys_power(0);
        }
    } while (key_char != 'h');

    if (use_panda) {
        syscall_sys_set_reporter(NULL);
    }

    if (syscall_process_state(usage_pid) < 4) {
        syscall_process_kill(usage_pid, 0);
    }

    syscall_kprint("\e[2J");

    kfree(envp);

    // unload all modules
    for (int i = 0; i < total; i++) {
        mod_t *mod = &mods_at_boot[i];
        syscall_pok_unload(mod->id);
    }

    syscall_kprint("all modules unloaded\n");

    syscall_mem_free_all(syscall_process_pid());

    return 0;
}
