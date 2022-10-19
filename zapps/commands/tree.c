#include <syscall.h>

void assemble_path(char old[], char new[], char result[]);
void shell_tree(char path[], int rec);

int main(int argc, char **argv) {
    int rec = 0; // param
    char *path = c_malloc(256);
    c_str_cpy(path, argv[1]);
    shell_tree(path, rec);
    c_free(path);
    return 0;
}

void shell_tree(char path[], int rec) {
    int elm_count = c_fs_get_folder_size(path);
    string_20_t *out_list = c_malloc(elm_count * sizeof(string_20_t));
    uint32_t *out_type = c_malloc(elm_count * sizeof(uint32_t));
    char tmp_path[256];
    c_fs_get_dir_content(c_fs_path_to_id(path, 0), out_list, out_type);
    for (int i = 0; i < elm_count; i++) out_type[i] = c_fs_type_sector(out_type[i]);
    for (int i = 0; i < elm_count; i++) {
        if (out_type[i] == 2) { // file
            for (int j = 0; j < rec; j++) c_fskprint("  ");
            c_fskprint("| $6%s\n", out_list[i].name);
        }
    }
    for (int i = 0; i < elm_count; i++) {
        if (out_type[i] == 3) { // folder
            for (int j = 0; j < rec; j++) c_fskprint("  ");
            assemble_path(path, out_list[i].name, tmp_path);
            c_fskprint("%s\n", out_list[i].name);
            shell_tree(tmp_path, rec + 1);
        }
    }
    if (rec == 0) c_fskprint("\n");
    c_free(out_list);
    c_free(out_type);
}

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; c_str_cpy(result, old);
    if (result[c_str_len(result) - 1] != '/') c_str_append(result, '/');
    for (int i = 0; i < c_str_len(new); i++) c_str_append(result, new[i]);
}