#include "syscall.h"

void print_status(char test_name[], int status);

int main(int argc, char **argv) {
    if (argc == 86) return 42;

    print_status("math pow()", c_pow(3, 7) == 2187 && c_pow(10, 3) == 1000);
    print_status("random", c_rand() != c_rand() || c_rand() != c_rand());
    print_status("rtc unix time", c_time_gen_unix() > c_time_get_boot());
    int old_active_alloc = c_mem_get_alloc_count() - c_mem_get_free_count();
    int * ptr = (int *) c_malloc(0x1000);
    print_status("memory alloc", ptr != 0);
    c_free(ptr);
    print_status("memory free", old_active_alloc == c_mem_get_alloc_count() - c_mem_get_free_count());
    print_status("file system", c_fs_get_file_size("/bin/tools/testall.bin") > 0);
    print_status("timer tick", c_timer_get_tick() > 0);
    print_status("run binary", c_sys_run_ifexist("/bin/tools/testall.bin", 0, NULL) == 42);
    return 0;
}

void print_status(char test_name[], int status) {
    char spaces[20]; int i;
    for (i = 0; i < 20 - c_str_len(test_name); i++)
        spaces[i] = ' ';
    spaces[i] = '\0';
    c_fskprint("$4%s%s%s\n", test_name, spaces, status ? "$7[$1done$7]" : "$7[$3fail$7]");
}
