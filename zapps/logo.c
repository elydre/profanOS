#include <stdint.h>
#include "addf.h"


int start(int addr, int arg) {
    INIT_AF(addr);

    AF_fs_declare_read_array();
    AF_fs_read_file();
    AF_mskprint();
    AF_free();

    char path[] = "/user/logo.itxt";
    uint32_t * data = fs_declare_read_array(path);
    char * str = fs_declare_read_array(path);
    
    fs_read_file(path, data);

    int i;
    for (i = 0; data[i] != (uint32_t) -1; i++)
        str[i] = (char) data[i];
    str[i] = '\n';

    mskprint(1, str);

    free(data);
    free(str);

    return arg;
}
