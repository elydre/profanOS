#include <syscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PROCESS_MAX 20

char *get_state(int state) {
    switch (state) {
        case 0: return "RUNNING";
        case 1: return "WAITING";
        case 2: return "TSLEEP";
        case 3: return "FSLEEP";
        case 4: return "ZOMBIE";
        case 5: return "DEAD";
        case 6: return "IDLE";
        default: return "UNKNOWN";
    }
}

void list_process(void) {
    int pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = c_process_generate_pid_list(pid_list, PROCESS_MAX);
    int pid;
    char name[64];
    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        c_process_get_name(pid, name);
        printf("pid: %d, ppid: %d, state: %s, name: %s, run: %gs, memory: %d Ko (%d alloc)\n",
                pid,
                c_process_get_ppid(pid),
                get_state(c_process_get_state(pid)),
                name,
                c_process_get_run_time(pid) / 1000.0,
                c_mem_get_info(8, pid) / 1024,
                c_mem_get_info(7, pid)
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

void show_help(int full) {
    puts("Usage: proc [mode] [pid]\n");
    if (!full) {
        puts("Use -h for more help\n");
        return;
    }
    puts("Modes:\n"
        "  -h: show this help\n"
        "  -l: list all processes\n"
        "  -k: kill a process\n"
        "  -s: asleep a process\n"
        "  -w: wake up a process\n"
    );
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
        show_help(0);
    else if (args.mode == MODE_FHLP)
        show_help(1);
    else if (args.mode == MODE_KILL)
        c_process_kill(args.pid);
    else if (args.mode == MODE_SLPP)
        c_process_sleep(args.pid, 0);
    else if (args.mode == MODE_WKUP)
        c_process_wakeup(args.pid);
    else return 1;
    return 0;
}
