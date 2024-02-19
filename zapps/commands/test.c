#include <syscall.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>

int main(void) {
    int pid = 0;
    pid = c_process_fork();

    printf("pid %d\n", pid);
    if (pid == 0) {
        printf("child %d\n", c_process_get_pid());
        exit(0);
    } else {
        printf("parent %d\n", c_process_get_pid());
    }
    profan_wait_pid(pid);
    return 0;
}
