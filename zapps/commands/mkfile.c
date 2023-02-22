#include <syscall.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("$BUsage: mkfile <name>\n");
        return 0;
    }

    if (!strcmp(argv[2], "..")) {
        printf("$3Un fichier ne peut pas avoir comme nom .. !\n");
    } else {
        c_fs_make_file(argv[1], argv[2]);
    }

    return 0;
}
