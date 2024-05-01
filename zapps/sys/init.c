/*****************************************************************************\
|   === init.c : 2024 ===                                                     |
|                                                                             |
|    First user-space program to run on boot                       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/libmmq.h>
#include <profan/panda.h>
#include <profan.h>

#define SHELL_PATH "/bin/fatpath/olivine.elf"
#define SHELL_NAME "olivine"

#define run_ifexist_pid(path, argc, argv, envp, pid) \
        run_ifexist_full((runtime_args_t){path, argc, argv, envp, 1}, pid)

typedef struct {
    int id;
    char path[256];
} mod_t;

mod_t mods_at_boot[] = {
    {1001, "/lib/mod/libmmq.bin"},
    {1002, "/lib/mod/filesys.bin"},
    {1003, "/lib/mod/devio.bin"},
    {1004, "/lib/mod/fmopen.bin"},
    {1005, "/lib/mod/profan.bin"},
    {1006, "/lib/mod/panda.bin"},
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
    if (c_dily_load(mod->path, mod->id)) {
        c_kprint("FAILED TO LOAD ");
        c_kprint(get_name(mod->path));
        c_kprint(" MODULE\n");
        return 1;
    }
    return 0;
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
    fd_putstr(1, c_sys_kinfo());
    fd_putstr(1, "\e[0m\n\n");
}

char wait_key(void) {
    while (c_kb_get_scfh());

    char key_char = 0;

    do {
        c_process_sleep(c_process_get_pid(), 50);
        key_char = profan_kb_get_char(c_kb_get_scfh(), 0);
    } while (key_char != 'g' && key_char != 'h' && key_char != 'j');

    return key_char;
}

char **envp;

void set_env(char *line) {
    if (envp == NULL) {
        envp = malloc(2 * sizeof(char *));
        envp[0] = line;
        envp[1] = NULL;
    } else {
        int i;
        for (i = 0; envp[i]; i++);
        envp = realloc(envp, (i + 2) * sizeof(char *));
        envp[i] = line;
        envp[i + 1] = NULL;
    }
}

int main(void) {
    char key_char, use_panda = 0;
    int sum, total, usage_pid;

    envp = NULL;

    total = (int) (sizeof(mods_at_boot) / sizeof(mod_t));
    sum = 0;

    for (int i = 0; i < total; i++) {
        sum += !print_load_status(i);
    }

    fd_printf(1, "Loaded %d/%d modules\n\n", sum, total);

    if (c_vesa_does_enable()) {
        panda_set_start(c_get_cursor_offset());
        use_panda = 1;
        if (fm_reopen(0, "/dev/panda")  < 0 ||
            fm_reopen(1, "/dev/panda")  < 0 ||
            fm_reopen(2, "/dev/pander") < 0 ||
            fm_reopen(3, "/dev/panda")  < 0 ||
            fm_reopen(4, "/dev/panda")  < 0 ||
            fm_reopen(5, "/dev/pander") < 0
        ) c_kprint("Failed to redirect to panda\n");
        set_env("TERM=/dev/panda");
        c_sys_set_reporter(userspace_reporter);
        run_ifexist_pid("/bin/tools/usage.elf", 0, NULL, NULL, &usage_pid);
    } else {
        c_kprint("[init] using kernel output for stdout\n");
        set_env("TERM=/dev/kterm");
    }

    welcome_print();

    do {
        set_env("PATH=/bin/cmd:/bin/fatpath");
        run_ifexist_pid(SHELL_PATH, 0, NULL, envp, NULL);

        fd_putstr(1, "[init] "SHELL_NAME" exited,\nAction keys:\n"
            " g - start "SHELL_NAME" again\n"
            " h - unload all modules and exit\n"
            " j - reboot profanOS\n"
        );

        if ((key_char = wait_key()) == 'j') {
            c_sys_reboot();
        }
    } while (key_char != 'h');

    if (use_panda) {
        c_sys_set_reporter(NULL);
    }

    c_kprint("\e[2J");
    if (c_process_get_state(usage_pid) < 4) {
        c_process_kill(usage_pid);
    }

    // unload all modules
    for (int i = 0; i < total; i++) {
        mod_t *mod = &mods_at_boot[i];
        c_dily_unload(mod->id);
    }

    free(envp);

    c_kprint("all modules unloaded\n");

    c_mem_free_all(c_process_get_pid());

    return 0;
}
