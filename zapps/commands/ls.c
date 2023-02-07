#include <syscall.h>
#include <stdlib.h>
#include <profan.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    char *path = malloc(256);
    char *ls_path = malloc(256);
    
    if (argc > 2 && strcmp(argv[2], "ls")) strcpy(path, argv[2]);
    else strcpy(path, "");

    if (path[0] == '\0') strcpy(ls_path, argv[1]);
    else assemble_path(argv[1], path, ls_path);

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
        if (elm_count) {
            free(out_types);
            free(out_ids);
        }
    }
    free(ls_path);
    free(path);
    return 0;
}
