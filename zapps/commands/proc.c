#include <syscall.h>
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

int main(void) {
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
    return 0;
}
