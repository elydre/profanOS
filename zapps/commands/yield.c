#include <syscall.h>
#include <i_string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    char *suffix = malloc(256);
    str_cpy(suffix, argv[2]);
    c_task_switch((str_cmp(suffix, "yield") == 0) ? 1 : ascii_to_int(suffix));
    free(suffix);
    return 0;
}
