#include <syscall.h>
#include <i_iolib.h>
#include <stdlib.h>

int main(int argc, char **argv) {

    char path[] = "/zada/logo.itxt";
    char *str = c_fs_declare_read_array(path);

    c_fs_read_file(path, (uint8_t *) str);

    msprint(1, str);

    free(str);

    return 0;
}
