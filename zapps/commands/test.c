#include <syscall.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>

int test(int val, char val2, uint32_t val3, char *val4) {
    printf("Test: %d, %c, %x, %s\n", val, val2, val3, val4);
    return 8;
} 

int main(int argc, char *argv[]) {
    int pid = c_process_create(test, 1, "test", 4, 42, 'c', 0xdeadbeef, "hello");
    c_process_wakeup(pid);
    printf("subprocess end with %d\n", profan_wait_pid(pid));
    return 0;
}
