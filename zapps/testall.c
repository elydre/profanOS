#include "addf.h"

void print_status(int addr, char test_name[], int status);

int main(int addr, int arg) {
    if (arg == 86) return 42;

    INIT_AF(addr);
    AF_mem_get_alloc_count();
    AF_mem_get_free_count();
    AF_fs_get_file_size();
    AF_sys_run_binary();
    AF_timer_get_tick();
    AF_time_get_boot();
    AF_time_gen_unix();
    AF_malloc();
    AF_free();
    AF_rand();
    AF_pow();

    print_status(addr, "math pow()", pow(3, 7) == 2187 && pow(10, 3) == 1000);
    print_status(addr, "random", rand() != rand() || rand() != rand());
    print_status(addr, "rtc unix time", time_gen_unix() > time_get_boot());
    int old_active_alloc = mem_get_alloc_count() - mem_get_free_count();
    int * ptr = (int *) malloc(0x1000);
    print_status(addr, "memory alloc", ptr != 0);
    free(ptr);
    print_status(addr, "memory free", old_active_alloc == mem_get_alloc_count() - mem_get_free_count());
    print_status(addr, "file system", fs_get_file_size("/bin/testall.bin") > 0);
    print_status(addr, "timer tick", timer_get_tick() > 0);
    print_status(addr, "run binary", sys_run_binary("/bin/testall.bin", 86) == 42);
    return 0;
}

void print_status(int addr, char test_name[], int status) {
    INIT_AF(addr);
    AF_fskprint();
    AF_str_len();
    char spaces[20]; int i;
    for (i = 0; i < 20 - str_len(test_name); i++)
        spaces[i] = ' ';
    spaces[i] = '\0';
    fskprint("$4%s%s%s\n", test_name, spaces, status ? "$7[$1done$7]" : "$7[$3fail$7]");
}
