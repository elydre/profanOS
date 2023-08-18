#include <syscall.h>
#include <stdio.h>

#define LOOPS 100000000

int main(int argc, char **argv) {
    c_process_set_sheduler(0);

    // get the time before the loop
    int start = c_timer_get_ms();

    // do a loop
    for (int i = 0; i < LOOPS; i++) {
        asm volatile("nop");
    }

    // get the time after the loop
    int end = c_timer_get_ms();

    c_process_set_sheduler(1);

    // print the time difference
    printf("loop time: %dms\n", end - start);
    printf("cpu speed: %dMHz\n", ((LOOPS / 1000) * 8 / (end - start)));

    return 0;
}
