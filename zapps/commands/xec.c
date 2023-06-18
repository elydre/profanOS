#include <syscall.h>
#include <profan.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: xec <name>\n");
        return 0;
    }

    char *full_path = malloc(256);
    assemble_path(argv[1], argv[2], full_path);

    // check if the file exists
    if (!c_fs_does_path_exists(full_path)) {
        printf("%s does not exist\n", full_path);
        free(full_path);
        return 0;
    }

    // read the file
    uint32_t size = c_fs_get_file_size(full_path);

    if (size < 0x1000) {
        printf("%s is too small\n", full_path);
        free(full_path);
        return 0;
    }

    uint8_t *data = malloc(size + 1);
    c_fs_read_file(full_path, data);

    // generate the new file name
    char *new_name = malloc(256);

    // remove the extension
    int len = strlen(full_path);
    int i;
    for (i = len - 1; i >= 0; i--) {
        if (full_path[i] == '.') {
            break;
        }
    }

    strncpy(new_name, full_path, i);
    new_name[i] = '\0';
    strcat(new_name, ".bin");

    printf("new file: %s\n", new_name);

    // get the file name
    char *file_name = NULL;
    char *parent_path = malloc(256);

    // isolate the file name and the parent path
    for (int i = len - 1; i >= 0; i--) {
        if (new_name[i] == '/') {
            file_name = new_name + i + 1;
            strncpy(parent_path, new_name, i);
            parent_path[i] = '\0';
            break;
        }
    }

    // theorically impossible
    if (file_name == NULL) {
        printf("$BInvalid file name\n");
    }

    // check if path exists and is directory
    else if (c_fs_does_path_exists(new_name) && c_fs_get_sector_type(c_fs_path_to_id(new_name)) == 3) {
        printf("$B%s is a directory\n", new_name);
    }

    else {
        if (!c_fs_does_path_exists(new_name)) {
            c_fs_make_file(parent_path, file_name);
        }
        c_fs_write_in_file(new_name, data + 0x1000, size - 0x1000);
    }

    free(parent_path);
    free(full_path);
    free(new_name);
    free(data);

    return 0;
}
