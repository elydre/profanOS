#include <syscall.h>
#include <i_iolib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void assemble_path(char old[], char new[], char result[]);

int main(int argc, char **argv) {
    char *current_dir = malloc(256);
    strcpy(current_dir, argv[1]);
    char *suffix = malloc(256);
    strcpy(suffix, argv[2]);
    char *path = malloc(256);

    if (strcmp(suffix, "ls"))
        strcpy(path, suffix);
    else strcpy(path, "");

    char ls_path[256];
    if (path[0] == '\0') strcpy(ls_path, current_dir);
    else assemble_path(current_dir, path, ls_path);

    if (!(c_fs_does_path_exists(ls_path) && c_fs_get_sector_type(c_fs_path_to_id(ls_path)) == 3)) {
        printf("$3%s$B is not a directory\n", ls_path);
    } else {
        int elm_count = c_fs_get_dir_size(ls_path);
        uint32_t *out_ids = malloc(elm_count * sizeof(uint32_t));
        int *out_types = malloc(elm_count * sizeof(int));
        char tmp_path[256], tmp_name[32];
        c_fs_get_dir_content(ls_path, out_ids);

        for (int i = 0; i < elm_count; i++) out_types[i] = c_fs_get_sector_type(out_ids[i]);
        for (int i = 0; i < elm_count; i++) {
            if (out_types[i] == 3) {
                c_fs_get_element_name(out_ids[i], tmp_name);
                printf("$2%s", tmp_name);
                for (unsigned int j = 0; j < 22 - strlen(tmp_name); j++) c_kprint(" ");
                assemble_path(ls_path, tmp_name, tmp_path);
                printf("%d elm\n", c_fs_get_dir_size(tmp_path));
            }
        } for (int i = 0; i < elm_count; i++) {
            if (out_types[i] == 2) {
                c_fs_get_element_name(out_ids[i], tmp_name);
                printf("$1%s", tmp_name);
                for (unsigned int j = 0; j < 22 - strlen(tmp_name); j++) c_kprint(" ");
                assemble_path(ls_path, tmp_name, tmp_path);
                printf("%d oct\n", c_fs_get_file_size(tmp_path));
            }
        }
        free(out_types);
        free(out_ids);
    }
    free(current_dir);
    free(suffix);
    free(path);
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
