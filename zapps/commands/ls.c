#include <syscall.h>
#include <i_string.h>
#include <i_iolib.h>
#include <i_mem.h>

void assemble_path(char old[], char new[], char result[]);

int main(int argc, char **argv) {
    char *current_dir = malloc(256);
    str_cpy(current_dir, argv[1]);
    char *suffix = malloc(256);
    str_cpy(suffix, argv[2]);
    char *path = malloc(256);

    if (str_cmp(suffix, "ls"))
        str_cpy(path, suffix);
    else str_cpy(path, "");

    char ls_path[256];
    if (path[0] == '\0') str_cpy(ls_path, current_dir);
    else assemble_path(current_dir, path, ls_path);

    if (!(c_fs_does_path_exists(ls_path) && c_fs_get_sector_type(c_fs_path_to_id(ls_path)) == 3)) {
        fsprint("$3%s$B is not a directory\n", ls_path);
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
                fsprint("$2%s", tmp_name);
                for (int j = 0; j < 22 - str_len(tmp_name); j++) c_kprint(" ");
                assemble_path(ls_path, tmp_name, tmp_path);
                fsprint("%d elm\n", c_fs_get_dir_size(tmp_path));
            }
        } for (int i = 0; i < elm_count; i++) {
            if (out_types[i] == 2) {
                c_fs_get_element_name(out_ids[i], tmp_name);
                fsprint("$1%s", tmp_name);
                for (int j = 0; j < 22 - str_len(tmp_name); j++) c_kprint(" ");
                assemble_path(ls_path, tmp_name, tmp_path);
                fsprint("%d oct\n", c_fs_get_file_size(tmp_path));
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
    result[0] = '\0'; str_cpy(result, old);
    if (result[str_len(result) - 1] != '/') str_append(result, '/');
    for (int i = 0; i < str_len(new); i++) str_append(result, new[i]);
}
