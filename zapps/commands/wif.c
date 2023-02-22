#include <syscall.h>
#include <i_iolib.h>
#include <string.h>
#include <profan.h>
#include <stdlib.h>


int main(int argc, char** argv) {
    if (argc < 3) {
        fsprint("$BUsage: wif <file>\n");
        return 0;
    }

    char *file = malloc(strlen(argv[1]) + strlen(argv[2]) + 2);
    assemble_path(argv[1], argv[2], file);

    if (c_fs_does_path_exists(file) && c_fs_get_sector_type(c_fs_path_to_id(file)) == 2) {
        char *char_content = malloc(256);

        c_kprint("-> ");
        input(char_content, 256, c_blue);
        c_kprint("\n");
    
        c_fs_write_in_file(file, (uint8_t *) char_content, strlen(char_content));
        free(char_content);
    } else {
        fsprint("$3%s$B file not found\n", file);
    }

    free(file);
    return 0;
}
