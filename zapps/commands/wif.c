#include <syscall.h>
#include <i_iolib.h>
#include <string.h>
#include <profan.h>
#include <stdlib.h>


int main(int argc, char** argv) {
    char *current_dir = malloc(256);
    char *suffix = malloc(256);
    strcpy(current_dir, argv[1]);
    strcpy(suffix, argv[2]);
    char *file = malloc(strlen(suffix)+strlen(current_dir)+2);
    assemble_path(current_dir, suffix, file);
    if (c_fs_does_path_exists(file) && c_fs_get_sector_type(c_fs_path_to_id(file)) == 2) {
        char char_content[70];
        fsprint("-> "); input(char_content, 70, c_blue); fsprint("\n");
        c_fs_write_in_file(file, (uint8_t *) char_content, strlen(char_content));
    } else fsprint("$3%s$B file not found\n", file);
    free(file);
    free(current_dir);
    free(suffix);
    return 0;
}
