#include <syscall.h>
#include <i_string.h>
#include <i_iolib.h>
#include <i_mem.h>

int main(int argc, char **argv) {
    char *suffix = malloc(256);
    str_cpy(suffix, argv[2]);
    char *current_dir = malloc(256);
    str_cpy(current_dir, argv[1]);
    if (!str_cmp(suffix, "..")) {
        fsprint("$3Un dossier ne peut pas avoir comme nom .. !\n");
    } else {
        c_fs_make_dir(current_dir, suffix);
    }
    free(current_dir);
    free(suffix);
    return 0;
}
