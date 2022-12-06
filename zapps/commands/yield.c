#include <syscall.h>

int main(int argc, char **argv) {
    char *suffix = c_malloc(256);
    c_str_cpy(suffix, argv[2]);
    c_task_switch((c_str_cmp(suffix, "yield") == 0) ? 1 : c_ascii_to_int(suffix));
    c_free(suffix);
    return 0;
}
