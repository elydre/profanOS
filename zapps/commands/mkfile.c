#include <syscall.h>
#include <string.h>
#include <mem.h>

int main(int argc, char **argv) {
    char *suffix = malloc(256);
    str_cpy(suffix, argv[2]);
    char *current_dir = malloc(256);
    str_cpy(current_dir, argv[1]);
    c_fs_make_file(current_dir, suffix);
    free(current_dir);
    free(suffix);
    return 0;
}
