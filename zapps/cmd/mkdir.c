#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("\e[31mUsage: \e[91mmkdir <name>\e[0m\n");
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *full_path = assemble_path(pwd, argv[1]);

    int len = strlen(full_path);

    // get the parent path
    char *parent_path = malloc(256);

    // isolate the parent path
    for (int i = len - 1; i >= 0; i--) {
        if (full_path[i] == '/') {
            strncpy(parent_path, full_path, i);
            parent_path[i] = '\0';
            break;
        }
    }

    if (parent_path[0] == '\0') {
        strcpy(parent_path, "/");
    }

    // chek if the parent directory exists
    sid_t parent_sid = fu_path_to_sid(ROOT_SID, parent_path);

    if (IS_NULL_SID(parent_sid) || !fu_is_dir(parent_sid)) {
        printf("\e[91m%s\e[31m is not a existing directory\e[0m\n", parent_path);
    }

    // check if the file already exists
    else if (!IS_NULL_SID(fu_path_to_sid(ROOT_SID, full_path))) {
        printf("\e[91m%s\e[31m already exists\e[0m\n", full_path);
    }

    // create the file
    else {
        fu_dir_create(0, full_path);
        free(parent_path);
        free(full_path);
        return 0;
    }

    free(parent_path);
    free(full_path);

    return 1;
}
