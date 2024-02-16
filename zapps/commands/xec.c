#include <syscall.h>
#include <filesys.h>
#include <profan.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("\e[31mUsage: \e[91mxec <input> <output>\e[0m\n");
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *input_file = assemble_path(pwd, argv[1]);

    sid_t input_sid = fu_path_to_sid(ROOT_SID, input_file);

    // check if the file exists
    if (IS_NULL_SID(input_sid)) {
        printf("\e[31mfile \e[91m%s\e[31m not found\e[0m\n", input_file);
        free(input_file);
        return 1;
    }

    // read the file
    uint32_t size = fu_get_file_size(input_sid);

    if (size < 0x1000) {
        printf("\e[31mfile \e[91m%s\e[31m is too small\e[0m\n", input_file);
        free(input_file);
        return 1;
    }

    uint8_t *data = malloc(size + 1);
    fu_file_read(input_sid, data, 0, size);
    data[size] = '\0';

    char *output_file = assemble_path(pwd, argv[2]);

    sid_t output_sid = fu_path_to_sid(ROOT_SID, output_file);

    if (!IS_NULL_SID(output_sid)) {
        fu_set_file_size(output_sid, size - 0x1000);
        fu_file_write(output_sid, data + 0x1000, 0, size - 0x1000);
        free(output_file);
        free(input_file);
        free(data);
        return 0;
    }

    // create the file
    output_sid = fu_file_create(0, output_file);
    if (IS_NULL_SID(output_sid)) {
        printf("\e[31mfailed to create file \e[91m%s\e[0m\n", output_file);
        free(output_file);
        free(input_file);
        free(data);
        return 1;
    }

    // write the file
    fu_set_file_size(output_sid, size - 0x1000);
    fu_file_write(output_sid, data + 0x1000, 0, size - 0x1000);

    free(output_file);
    free(input_file);
    free(data);
    return 0;
}
