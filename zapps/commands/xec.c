#include <syscall.h>
#include <filesys.h>
#include <profan.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4 || (argc == 4 && strcmp(argv[3], "-t"))) {
        puts("Usage: xec <input> <output> [-t]\n"
            "  add -t to use with tcc instead of vlink\n"
        );
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *input_file = assemble_path(pwd, argv[1]);

    sid_t input_sid = fu_path_to_sid(ROOT_SID, input_file);

    // check if the file exists
    if (IS_NULL_SID(input_sid)) {
        printf("file %s not found\n", input_file);
        free(input_file);
        return 1;
    }

    // read the file
    uint32_t size = fu_get_file_size(input_sid);

    if ((argc == 4 && size < 0x184) || (argc == 3 && size < 0x1000)) {
        printf("file %s is too small\n", input_file);
        free(input_file);
        return 1;
    }

    uint8_t *data = malloc(size + 1);
    fu_file_read(input_sid, data, 0, size);
    data[size] = '\0';

    char *output_file = assemble_path(pwd, argv[2]);

    sid_t output_sid = fu_path_to_sid(ROOT_SID, output_file);

    if (IS_NULL_SID(output_sid)) {
        output_sid = fu_file_create(0, output_file);
    }

    // create the file
    if (IS_NULL_SID(output_sid)) {
        printf("failed to create file %s\n", output_file);
        free(output_file);
        free(input_file);
        free(data);
        return 1;
    }

    if (argc == 3) {
        // vlink
        fu_set_file_size(output_sid, size - 0x1000);
        fu_file_write(output_sid, data + 0x1000, 0, size - 0x1000);
    } else {
        // tcc
        uint8_t *zero = calloc(1, 0x2000 + 0x184);
        fu_set_file_size(output_sid, 0x2000 + size);
        fu_file_write(output_sid, zero, 0, 0x2000 + 0x184);
        fu_file_write(output_sid, data + 0x184, 0x2000 + 0x184, size - 0x184);
        free(zero);
    }

    free(output_file);
    free(input_file);
    free(data);
    return 0;
}
