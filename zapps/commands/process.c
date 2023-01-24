#include <syscall.h>
#include <stdlib.h>
#include <profan.h>
#include <string.h>
#include <stdio.h>

void process1();
void process2();

int main(int argc, char **argv) {
    printf("Multiprocessing test\n");
    // we need to create the processes
    int pid1 = c_process_create(process1, "process1");
    int pid2 = c_process_create(process2, "process2");
    // we awake them
    c_process_wakeup(pid1);
    c_process_wakeup(pid2);

    while (1) {
        printf("Main process\n");
    }

    return 0;
}

void process1() {
    while (1) {
        printf("Process 1\n");
    }
}

void process2() {
    while (1) {
        printf("Process 2\n");
    }
}