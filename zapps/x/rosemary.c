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

// @LINK: libmmq

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/libmmq.h>
#include <profan/panda.h>
#include <profan/clip.h>
#include <profan.h>

#include <fcntl.h> // open flags

#define LOADER_NAME "rosemary"

#define SHELL_PATH "/bin/f/olivine.elf"
#define SHELL_NAME "olivine"

#define START_USAGE_GRAPH 1

typedef struct {
    int id;
    char *path;
} mod_t;

mod_t mods_at_boot[] = {
    {1, "/lib/mod/filesys.pkm"},
    {2, "/lib/mod/devio.pkm"},
    {3, "/lib/mod/fmopen.pkm"},
    {4, "/lib/mod/profan.pkm"},
    {5, "/lib/mod/panda.pkm"},
    {6, "/lib/mod/ata.pkm"},
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
    int error_code = syscall_mod_load(mod->path, mod->id);

    if (error_code > 0)
        return 0;

    syscall_kprint("["LOADER_NAME"] ");
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
        mmq_putstr(1, tmp);
    }
}

void welcome_print(void) {
    rainbow_print("Welcome to profanOS!\n\n");
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
        envp = mmq_malloc(2 * sizeof(char *));
        envp[0] = line;
        envp[1] = NULL;
    } else {
        int i;
        for (i = 0; envp[i]; i++);
        envp = mmq_realloc(envp, (i + 2) * sizeof(char *));
        envp[i] = line;
        envp[i + 1] = NULL;
    }
}

int main(void) {
    runtime_args_t args;
    int total, usage_pid;
    char key_char;

    envp = NULL;

    total = (int) (sizeof(mods_at_boot) / sizeof(mod_t));

    for (int i = 0; i < total; i++) {
        print_load_status(i);
    }

    mmq_printf(1, "Successfully loaded %d modules\n\n", total);

    if (syscall_vesa_state()) {
        panda_sync_start();
        if (fm_reopen(0, "/dev/panda", O_RDONLY)  < 0 ||
            fm_reopen(1, "/dev/panda", O_WRONLY)  < 0 ||
            fm_reopen(2, "/dev/pander", O_WRONLY) < 0
        ) syscall_kprint("["LOADER_NAME"] Failed to redirect to panda\n");
        set_env("TERM=/dev/panda");
        if (START_USAGE_GRAPH) {
            args = (runtime_args_t){
                .path = "/bin/g/usage.elf",
                .wd = NULL,
                .argc = 1,
                .argv = (char *[]){"usage"},
                .envp = NULL,
                .sleep_mode = 0
            };
            run_ifexist(&args, &usage_pid);
        }

    } else {
        set_env("TERM=/dev/kterm");
    }

    welcome_print();

    set_env("PATH=/bin/c:/bin/f:/bin/l");
    set_env("DEFRUN=/bin/f/tcc.elf -run");
    set_env("HOME=/");
    set_env("PWD=/");

    do {
        args = (runtime_args_t){
            .path = SHELL_PATH,
            .wd = NULL,
            .argc = 1,
            .argv = (char *[]) {SHELL_NAME},
            .envp = envp,
            .sleep_mode = 1
        };

        run_ifexist(&args, &usage_pid);

        mmq_putstr(1, "\n["LOADER_NAME"] "SHELL_NAME" exited,\nAction keys:\n"
            " g - start "SHELL_NAME" again\n"
            " h - unload all modules and exit\n"
            " j - reboot profanOS\n"
        );

        if ((key_char = wait_key()) == 'j') {
            syscall_sys_power(0);
        }
    } while (key_char != 'h');

    if (syscall_process_state(usage_pid) < 4) {
        syscall_process_kill(usage_pid, 0);
    }

    syscall_kprint("\e[2J");

    mmq_free(envp);

    // unload all modules
    for (int i = 0; i < total; i++) {
        mod_t *mod = &mods_at_boot[i];
        syscall_mod_unload(mod->id);
    }

    syscall_kprint("all modules unloaded\n");

    return 0;
}
