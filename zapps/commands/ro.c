#include <syscall.h>
#include <stdlib.h>
#include <profan.h>
#include <string.h>
#include <stdio.h>

#define MAGIC_ADDR 0x6400000

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: ro <file>\n");
        return 1;
    }

    char *new_path = malloc(strlen(argv[1]) + strlen(argv[2]) + 2);
    assemble_path(argv[1], argv[2], new_path);

    if (!(c_fs_does_path_exists(new_path) && c_fs_get_sector_type(c_fs_path_to_id(new_path)) == 2)) {
        printf("%s is not a file\n", new_path);
        free(new_path);
        return 1;
    }

    printf("copying %s to %x\n", new_path, MAGIC_ADDR);
    c_fs_read_file(new_path, (uint8_t *) MAGIC_ADDR);
    free(new_path);

    printf("executing %x\n", MAGIC_ADDR);
    return ((int (*)(int, char**)) MAGIC_ADDR)(argc - 2, argv + 2);
}
