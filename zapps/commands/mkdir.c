#include <syscall.h>
#include <profan.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("$BUsage: $3mkdir <name>\n");
        return 0;
    }

    char *full_path = malloc(256);
    assemble_path(argv[1], argv[2], full_path);

    int len = strlen(full_path);

    // get the dir name
    char *dir_name = NULL;
    char *parent_path = malloc(256);

    // isolate the dir name and the parent path
    for (int i = len - 1; i >= 0; i--) {
        if (full_path[i] == '/') {
            dir_name = full_path + i + 1;
            strncpy(parent_path, full_path, i);
            parent_path[i] = '\0';
            break;
        }
    }

    // theorically impossible
    if (dir_name == NULL) {
        printf("$BInvalid dir name\n");
    }

    // chek if the parent directory exists
    else if (!(c_fs_does_path_exists(parent_path) && c_fs_get_sector_type(c_fs_path_to_id(parent_path)) == 3)) {
        printf("$3%s$B is not a existing directory\n", parent_path);
    }

    // check if the dir already exists
    else if (c_fs_does_path_exists(full_path)) {
        printf("$3%s$B already exists\n", full_path);
    }

    // create the dir
    else {
        c_fs_make_dir(parent_path, dir_name);
    }

    free(parent_path);
    free(full_path);

    return 0;
}
