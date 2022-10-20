#include <syscall.h>

int main(int argc, char **argv) {
    char *suffix = c_malloc(256);
    c_str_cpy(suffix, argv[2]);
    char *current_dir = c_malloc(256);
    c_str_cpy(current_dir, argv[1]);
    c_fs_make_file(current_dir, suffix);
    c_free(current_dir);
    c_free(suffix);
    return 0;
}
