#include <syscall.h>
#include <stdio.h>

#define PROCESS_MAX 20

int main(int argc, char **argv) {
    char *state[] = {
        "RUNNING",
        "WAITING",
        "SLEEPING",
        "ZOMBIE",
    };
    int pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = c_process_generate_pid_list(pid_list, PROCESS_MAX);
    int pid;
    char name[64];
    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        c_process_get_name(pid, name);
        printf("pid: %d, ppid: %d, state: %s, name: %s, memory: %d Ko (%d alloc)\n",
                pid,
                c_process_get_ppid(pid),
                state[c_process_get_state(pid)],
                name,
                c_mem_get_info(8, pid) / 1024,
                c_mem_get_info(7, pid)
        );
    }
    return 0;
}
