#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <profan.h>

int main() {
    void *addr = malloc(1024);
    printf("malloc(1024) = %p\n", addr);
    assert(addr != NULL);
    strcpy(addr, "Hello, world!");

    int pid = fork();

    if (pid == 0) {
        printf("child: %s\n", addr);
        memcpy(addr, "Goodbye, world!", 16);
        printf("child: %s\n", addr);
        free(addr);
        exit(0);
    } else {
        profan_wait_pid(pid);
    }

    printf("parent: %s\n", addr);
    free(addr);
    return 0;
}
