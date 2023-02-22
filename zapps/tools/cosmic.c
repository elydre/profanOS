#include <syscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <i_time.h>

uint32_t rand_value = 0;

uint32_t local_rand() {
    rand_value = rand_value * 1103515245 + 12345;
    return (uint32_t) (rand_value / 65536) % 32768;
}

int main(int argc, char **argv) {
    printf("sarting cosmic ray simulator!\n");
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    rand_value = time_gen_unix();

    uint32_t addr, value;

    while (1) {
        addr = local_rand() << 12 | local_rand() & 0xFFF;
        value = local_rand();
        *(uint32_t *) addr = value;
        ms_sleep(1);
    }
}
