#include <syscall.h>
#include <profan.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("$BUsage: $3xec <input> <output>$$\n");
        return 0;
    }

    char *input_file = malloc(256);
    assemble_path(argv[1], argv[2], input_file);

    // check if the file exists
    if (!c_fs_does_path_exists(input_file)) {
        printf("$Bfile $3%s$B not found$$\n", input_file);
        free(input_file);
        return 0;
    }

    // read the file
    uint32_t size = c_fs_get_file_size(input_file);
    uint8_t *data = malloc(size + 1);
    c_fs_read_file(input_file, data);

    if (size < 0x1000) {
        printf("$Bfile $3%s$B is too small$$\n", input_file);
        free(input_file);
        free(data);
        return 0;
    }

    char *output_file = malloc(256);
    assemble_path(argv[1], argv[3], output_file);

    char *parent_dir = malloc(strlen(output_file) + 1);
    char *file_name = malloc(strlen(output_file) + 1);

    // isolate parent directory and file name
    int i = strlen(output_file) - 1;
    for (; output_file[i] != '/'; i--);
    strncpy(parent_dir, output_file, i);
    parent_dir[i] = '\0';
    strcpy(file_name, output_file + i + 1);

    // check if path exists and is directory
    if (c_fs_does_path_exists(output_file) && c_fs_get_sector_type(c_fs_path_to_id(output_file)) == 3) {
        printf("$3%s$B is a directory$$\n", output_file);
    } else {
        if (!c_fs_does_path_exists(output_file)) {
            c_fs_make_file(parent_dir, file_name);
        }
        c_fs_write_in_file(output_file, data + 0x1000, size - 0x1000);
    }

    free(output_file);
    free(input_file);

    free(parent_dir);
    free(file_name);

    free(data);

    return 0;
}
