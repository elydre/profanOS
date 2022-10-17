#include "syscall.h"

int main(int arg) {

    char path[80];
    c_fskprint("Entrez le chemin du fichier : ");
    c_input(path, 80, 0x09);
    c_fskprint("\n");
    uint32_t *data_uint32 = c_fs_declare_read_array(path);
    char *data_char = c_fs_declare_read_array(path);
    c_fs_read_file(path, data_uint32);
    int i;
    for (i=0; data_uint32[i] != (uint32_t) -1; i++) {
        data_char[i] =  data_uint32[i];
    } i++; data_char[i] = '\0';
    c_free(data_uint32);
    c_fskprint("%s\n", data_char);

    c_free(data_char);
    return arg;
}