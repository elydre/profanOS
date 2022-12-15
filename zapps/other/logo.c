#include "syscall.h"

int main(int argc, char **argv) {

    char path[] = "/zada/logo.itxt";
    char *str = c_fs_declare_read_array(path);

    c_fs_read_file(path, (uint8_t *) str);

    c_mskprint(1, str);

    c_free(str);

    PATH_EXIT();

    return 0;
}
