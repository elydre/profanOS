/*****************************************************************************\
|   === proc.c : 2024 ===                                                     |
|                                                                             |
|    Basic command line process management tool                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan/panda.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define PROCESS_MAX 64

char *get_state(int state) {
    switch (state) {
        case 0: return "free";
        case 1: return "zombie";
        case 2: return "sleeping";
        case 3: return "in queue";
        case 4: return "running";
        case 5: return "idle";
        default: return "unknown";
    }
}

void sort_tab(uint32_t *tab, int size) {
    uint32_t tmp;
    for (int i = 0; i < size; i++) {
        for (int j = i; j > 0 && tab[j] < tab[j - 1]; j--) {
            tmp = tab[j];
            tab[j] = tab[j - 1];
            tab[j - 1] = tmp;
        }
    }
}

void str_insert(char *str, char *insert, int pos) {
    int len = strlen(str);
    int ilen = strlen(insert);
    if (pos > len)
        pos = len;
    if (pos < 0)
        pos = 0;
    memmove(str + pos + ilen, str + pos, len - pos + 1);
    memcpy(str + pos, insert, ilen);
}

void draw_line(char *buf, int percent) {
    int tmp;

    if (percent > 100)
        percent = 100;

    tmp = strlen(buf);
    memmove(buf + 51 - tmp, buf, tmp);
    memset(buf, ' ', 51 - tmp);
    buf[0] = '[';
    for (int i = 0; i < 50; i++) {
        if (i > percent / 2 || buf[i + 1] != ' ')
            break;
        buf[i + 1] = '|';
    }
    buf[51] = ']';
    buf[52] = '\0';

    // add colors
    str_insert(buf, "\e[0m", (percent == 100 ? 99: percent) / 2 + 2);
    str_insert(buf, "\e[31m", 1);
}

void list_process(int mode) {
    uint32_t pid_list[PROCESS_MAX]; // it's a define
    char *printf_format;

    int pid_list_len = syscall_process_list_all(pid_list, PROCESS_MAX);
    sort_tab(pid_list, pid_list_len);

    if (mode == 1) {
        printf("   profanOS . kernel %s . %d processes . uptime %ds (%ds IDLE)  \n",
                syscall_sys_kinfo(),
                pid_list_len - 2,
                syscall_timer_get_ms() / 1000,
                syscall_process_run_time(1) / 1000
        );

        puts("\e[47m\e[30m PID PPID     STATE     TIME      MEM ALLOC NAME                                \e[0m");
        printf_format = "%4d %4d %9s %7gs %7dK %5d %-36s\n";
    } else if (mode == 2) {
        printf_format = "%d %d %s %g %d %d %s\n";
    } else {
        puts(" PID PPID     STATE     TIME      MEM ALLOC NAME");
        printf_format = "%4d %4d %9s %7gs %7dK %5d %s\n";
    }

    int pid;
    char *name;
    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        if (pid == 1) continue;
        name = (char *) syscall_process_info(pid, PROC_INFO_NAME, NULL);
        printf(printf_format,
                pid,
                syscall_process_ppid(pid),
                get_state(syscall_process_state(pid)),
                syscall_process_run_time(pid) / 1000.0,
                syscall_mem_info(12, pid) / 1024,
                syscall_mem_info(11, pid),
                name
        );
    }

    if (mode == 1)
        printf("%80s\n", "");
}

void top_loop(void) {
    int last_idle, idle, last_total, total;
    int total_mem, used_mem, cpu;
    char buf[64];
    idle = total = 0;

    void *screen_bk = panda_screen_backup();

    // clear screen
    printf("\e[2J");
    fflush(stdout);

    while (syscall_sc_get());

    do {
        // move cursor to top left
        printf("\e[H");

        last_idle = idle;
        last_total = total;

        idle = syscall_process_run_time(1);
        total = syscall_timer_get_ms();

        cpu = total - last_total;
        cpu = 100 - (idle - last_idle) * 100 / (cpu ? cpu : 1);
        if (cpu > 100)
            cpu = 100;

        tm_t t;
        syscall_time_get(&t);

        total_mem = syscall_mem_info(0, 0) / 1024;
        used_mem = syscall_mem_info(6, 0) / 1024;

        sprintf(buf, "%d%%", cpu);
        draw_line(buf, cpu);
        printf("%s %02d:%02d:%02d UTC\n", buf, t.tm_hour, t.tm_min, t.tm_sec);

        sprintf(buf, "%dM/%dM", used_mem / 1024, total_mem / 1024);
        draw_line(buf, used_mem * 100 / total_mem);
        printf("%s %d allocs    \n\n", buf, syscall_mem_info(4, 0) - syscall_mem_info(5, 0));
        list_process(1);
        for (int i = 0; i < 9; i++) {
            if (syscall_sc_get()) {
                last_idle = -1;
                break;
            }
            syscall_process_sleep(syscall_process_pid(), 100);
        }
    } while (last_idle != -1);

    panda_screen_restore(screen_bk);
    panda_screen_free(screen_bk);
}

typedef struct {
    int mode;
    int pid;
} proc_args_t;

#define MODE_LIST 0
#define MODE_LHLP 1
#define MODE_FHLP 2
#define MODE_EXIT 3
#define MODE_SLPP 4
#define MODE_WKUP 5
#define MODE_TOPL 6
#define MODE_PARS 7

int show_help(void) {
    puts(
        "Usage: proc [mode] [pid]\n"
        "Modes:\n"
        "  -h: show this help\n"
        "  -l: list processes (default)\n"
        "  -k: kill a process\n"
        "  -p: like -l but parser friendly\n"
        "  -s: asleep a process\n"
        "  -t  top like interface\n"
        "  -w: wake up a process"
    );
    return 0;
}

proc_args_t parse_args(int argc, char **argv) {
    proc_args_t args;
    args.mode = MODE_LIST;
    args.pid = -1;
    if (argc == 1) {
        return args;
    }
    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0) {
            args.mode = MODE_FHLP;
            return args;
        }
        if (strcmp(argv[1], "-l") == 0) {
            args.mode = MODE_LIST;
            return args;
        }
        if (strcmp(argv[1], "-t") == 0) {
            args.mode = MODE_TOPL;
            top_loop();
            return args;
        }
        if (strcmp(argv[1], "-p") == 0) {
            args.mode = MODE_PARS;
            return args;
        }
        fprintf(stderr, "proc: invalid option -- '%s'\n", argv[1]);
        args.mode = MODE_LHLP;
        return args;
    }
    else if (argc == 3) {
        if (strcmp(argv[1], "-k") == 0) {
            args.mode = MODE_EXIT;
        }
        else if (strcmp(argv[1], "-s") == 0) {
            args.mode = MODE_SLPP;
        }
        else if (strcmp(argv[1], "-w") == 0) {
            args.mode = MODE_WKUP;
        }
        else {
            fprintf(stderr, "proc: invalid option -- '%s'\n", argv[1]);
            args.mode = MODE_LHLP;
            return args;
        }
        args.pid = atoi(argv[2]);
        return args;
    }
    fputs("Usage: proc [mode] [pid]\n", stderr);
    args.mode = MODE_LHLP;
    return args;
}

int main(int argc, char **argv) {
    proc_args_t args = parse_args(argc, argv);

    if (args.mode == MODE_LHLP) {
        fputs("Try 'proc -h' for more information.\n", stderr);
        return 1;
    }

    if (args.mode == MODE_LIST)
        list_process(0);
    else if (args.mode == MODE_TOPL)
        top_loop();
    else if (args.mode == MODE_PARS)
        list_process(2);
    else if (args.mode == MODE_FHLP)
        return show_help();
    else if (args.mode == MODE_EXIT)
        syscall_process_kill(args.pid, 1);
    else if (args.mode == MODE_SLPP)
        syscall_process_sleep(args.pid, UINT32_MAX);
    else if (args.mode == MODE_WKUP)
        syscall_process_wakeup(args.pid, 0);
    else return 1;
    return 0;
}
