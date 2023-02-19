#include <syscall.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void print_status(char test_name[], int status);

int main(int argc, char **argv) {
    if (argc == 86) return 42;

    print_status("random", c_rand() != c_rand() || c_rand() != c_rand());
    int old_active_alloc = c_mem_get_info(4, 0) - c_mem_get_info(5, 0);
    int *ptr = (int *) malloc(0x1000);
    print_status("memory alloc", ptr != 0);
    free(ptr);
    print_status("memory free", old_active_alloc == c_mem_get_info(4, 0) - c_mem_get_info(5, 0));
    print_status("file system", c_fs_get_file_size("/bin/tools/testall.bin") > 0);
    print_status("timer tick", c_timer_get_ms() > 0);
    print_status("run binary", c_run_ifexist("/bin/tools/testall.bin", 86, NULL) == 42);
    return 0;
}

void print_status(char test_name[], int status) {
    char spaces[20]; 
    unsigned int i;
    for (i = 0; i < 20 - strlen(test_name); i++)
        spaces[i] = ' ';
    spaces[i] = '\0';
    printf("$4%s%s%s\n", test_name, spaces, status ? "$7[$1done$7]" : "$7[$3fail$7]");
}
