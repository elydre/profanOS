#include <syscall.h>
#include <iolib.h>

int main(int argc, char **argv) {

    char path[] = "/zada/logo.itxt";
    uint32_t *data = c_fs_declare_read_array(path);
    char *str = c_fs_declare_read_array(path);
    
    c_fs_read_file(path, data);

    int i;
    for (i = 0; data[i] != (uint32_t) -1; i++)
        str[i] = (char) data[i];
    str[i] = '\n';

    mskprint(1, str);

    c_free(data);
    c_free(str);

    PATH_EXIT();

    return 0;
}
