#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void assemble_path(char old[], char new[], char result[]);;

int main(int argc, char **argv) {
    char *fpath = malloc(256);
    char *suffix = malloc(256);
    strcpy(fpath, argv[1]);
    strcpy(suffix, argv[2]);
    char *file = malloc(strlen(suffix) + strlen(fpath) + 2);
    assemble_path(fpath, suffix, file);
    if (c_fs_does_path_exists(file) && c_fs_get_sector_type(c_fs_path_to_id(file)) == 2) {
        char *char_content = c_fs_declare_read_array(file);
        c_fs_read_file(file, (uint8_t *) char_content);
        c_ckprint(char_content, c_magenta);
        printf("\n");
        free(char_content);
    } else printf("$3%s$B file not found\n", file);
    free(file);
    free(fpath);
    free(suffix);
    return 0;
}

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; strcpy(result, old);
    if (result[strlen(result) - 1] != '/') {
        strncat(result, "/", 1);
    }
    for (unsigned int i = 0; i < strlen(new); i++) {
        strncat(result, &new[i], 1);
    }
}
