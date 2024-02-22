#include <syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>

int main(void) {
    int pid = 0;
    pid = fork();

    printf("pid %d\n", pid);
    if (pid == 0) {
        execl("/bin/commands/echo.bin", "echo", "hello", "world", NULL);
        printf("child?\n");
        exit(0);
    } else {
        printf("parent %d\n", getpid());
    }
    printf("waiting for pid %d\n", pid);
    profan_wait_pid(pid);
    printf("done waiting for pid %d\n", pid);
    return 0;
}
