#include <syscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PROCESS_MAX 20

char *get_state(int state) {
    switch (state) {
        case 0: return "running";
        case 1: return "waiting";
        case 2: return "Tsleep";
        case 3: return "Fsleep";
        case 4: return "zombie";
        case 5: return "dead";
        case 6: return "idle";
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

void list_process(void) {
    uint32_t pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = c_process_generate_pid_list(pid_list, PROCESS_MAX);
    sort_tab(pid_list, pid_list_len);

    puts(" PID PPID     STATE     TIME      MEM ALLOC NAME");
    int pid;
    char name[64];
    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        c_process_get_name(pid, name);
        printf("%4d %4d %9s %7gs %7dK %5d %-36s\n",
                pid,
                c_process_get_ppid(pid),
                get_state(c_process_get_state(pid)),
                c_process_get_run_time(pid) / 1000.0,
                c_mem_get_info(8, pid) / 1024,
                c_mem_get_info(7, pid),
                name
        );
    }
}

typedef struct {
    int mode;
    int pid;
} proc_args_t;

#define MODE_LIST 0
#define MODE_LHLP 1
#define MODE_FHLP 2
#define MODE_KILL 3
#define MODE_SLPP 4
#define MODE_WKUP 5

int show_help(int full) {
    puts("Usage: proc [mode] [pid]");
    if (!full) {
        puts("Use -h for more help");
        return 1;
    }
    puts("Modes:\n"
        "  -h: show this help\n"
        "  -l: list all processes\n"
        "  -k: kill a process\n"
        "  -s: asleep a process\n"
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
        args.mode = MODE_LHLP;
        return args;
    }
    else if (argc == 3) {
        if (strcmp(argv[1], "-k") == 0) {
            args.mode = MODE_KILL;
        }
        else if (strcmp(argv[1], "-s") == 0) {
            args.mode = MODE_SLPP;
        }
        else if (strcmp(argv[1], "-w") == 0) {
            args.mode = MODE_WKUP;
        }
        else {
            args.mode = MODE_LHLP;
            return args;
        }
        args.pid = atoi(argv[2]);
        return args;
    }
    args.mode = MODE_LHLP;
    return args;
}

int main(int argc, char **argv) {
    proc_args_t args = parse_args(argc, argv);

    if (args.mode == MODE_LIST)
        list_process();
    else if (args.mode == MODE_LHLP)
        return show_help(0);
    else if (args.mode == MODE_FHLP)
        return show_help(1);
    else if (args.mode == MODE_KILL)
        c_process_kill(args.pid);
    else if (args.mode == MODE_SLPP)
        c_process_sleep(args.pid, 0);
    else if (args.mode == MODE_WKUP)
        c_process_wakeup(args.pid);
    else return 1;
    return 0;
}
