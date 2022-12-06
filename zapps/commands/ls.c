#include <syscall.h>

void assemble_path(char old[], char new[], char result[]);

int main(int argc, char **argv) {
    char *current_dir = c_malloc(256);
    c_str_cpy(current_dir, argv[1]);
    char *suffix = c_malloc(256);
    c_str_cpy(suffix, argv[2]);
    char *path = c_malloc(256);

    if (c_str_cmp(suffix, "ls"))
        c_str_cpy(path, suffix);
    else c_str_cpy(path, "");

    char ls_path[256];
    if (path[0] == '\0') c_str_cpy(ls_path, current_dir);
    else assemble_path(current_dir, path, ls_path);

    if (!(c_fs_does_path_exists(ls_path) && c_fs_type_sector(c_fs_path_to_id(ls_path, 0)) == 3)) {
        c_fskprint("$3%s$B is not a directory\n", ls_path);
    } else {
        int elm_count = c_fs_get_folder_size(ls_path);
        string_20_t *out_list = c_malloc(elm_count * sizeof(string_20_t));
        uint32_t *out_type = c_malloc(elm_count * sizeof(uint32_t));
        char tmp_path[256];
        c_fs_get_dir_content(c_fs_path_to_id(ls_path, 0), out_list, out_type);

        for (int i = 0; i < elm_count; i++) out_type[i] = c_fs_type_sector(out_type[i]);
        for (int i = 0; i < elm_count; i++) {
            if (out_type[i] == 3) {
                c_fskprint("$2%s", out_list[i].name);
                for (int j = 0; j < 22 - c_str_len(out_list[i].name); j++) c_fskprint(" ");
                assemble_path(ls_path, out_list[i].name, tmp_path);
                c_fskprint("%d elm\n", c_fs_get_folder_size(tmp_path));
            }
        } for (int i = 0; i < elm_count; i++) {
            if (out_type[i] == 2) {
                c_fskprint("$1%s", out_list[i].name);
                for (int j = 0; j < 22 - c_str_len(out_list[i].name); j++) c_fskprint(" ");
                assemble_path(ls_path, out_list[i].name, tmp_path);
                c_fskprint("%d sect\n", c_fs_get_file_size(tmp_path));
            }
        }
        c_free(out_list);
        c_free(out_type);
    }
    c_free(current_dir);
    c_free(suffix);
    c_free(path);
    return 0;
}

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; c_str_cpy(result, old);
    if (result[c_str_len(result) - 1] != '/') c_str_append(result, '/');
    for (int i = 0; i < c_str_len(new); i++) c_str_append(result, new[i]);
}
