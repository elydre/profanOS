#include <syscall.h>
#include <i_iolib.h>
#include <i_string.h>
#include <stdlib.h>
#include <string.h>

void assemble_path(char old[], char new[], char result[]);;

int main(int argc, char **argv) {
    strsignal(0);
    char *fpath = malloc(256);
    char *suffix = malloc(256);
    str_cpy(fpath, argv[1]);
    str_cpy(suffix, argv[2]);
    char *file = malloc(str_len(suffix) + str_len(fpath) + 2);
    assemble_path(fpath, suffix, file);
    if (c_fs_does_path_exists(file) && c_fs_get_sector_type(c_fs_path_to_id(file)) == 2) {
        char *char_content = c_fs_declare_read_array(file);
        c_fs_read_file(file, (uint8_t *) char_content);
        c_ckprint(char_content, c_magenta);
        fsprint("\n");
        free(char_content);
    } else fsprint("$3%s$B file not found\n", file);
    free(file);
    free(fpath);
    free(suffix);
    return 0;
}

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; str_cpy(result, old);
    if (result[str_len(result) - 1] != '/') str_append(result, '/');
    for (int i = 0; i < str_len(new); i++) str_append(result, new[i]);
}
