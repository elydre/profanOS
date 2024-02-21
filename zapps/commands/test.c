#include <syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>

int main(void) {
    int pid = 0;
    pid = c_process_fork();

    printf("pid %d\n", pid);
    if (pid == 0) {
        printf("child %d\n", getpid());
    } else {
        printf("parent %d\n", getpid());
    }
    printf("waiting for pid %d\n", pid);
    profan_wait_pid(pid);
    printf("done waiting for pid %d\n", pid);
    exit(0);
    return 0;
}
