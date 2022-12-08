#include <syscall.h>
#include <iolib.h>

void assemble_path(char old[], char new[], char result[]);;

int main(int argc, char **argv) {
    char *fpath = c_malloc(256);
    char *suffix = c_malloc(256);
    c_str_cpy(fpath, argv[1]);
    c_str_cpy(suffix, argv[2]);
    char *file = c_malloc(c_str_len(suffix) + c_str_len(fpath) + 2);
    assemble_path(fpath, suffix, file);
    if (c_fs_does_path_exists(file) && c_fs_type_sector(c_fs_path_to_id(file, 0)) == 2) {
        uint32_t *file_content = c_fs_declare_read_array(file);
        char *char_content = c_fs_declare_read_array(file);
        c_fs_read_file(file, file_content);
        int char_count;
        for (char_count = 0; file_content[char_count] != (uint32_t) -1; char_count++)
            char_content[char_count] = (char) file_content[char_count];
        char_content[char_count] = '\0';
        c_ckprint(char_content, c_magenta);
        fskprint("\n");
        c_free(file_content);
        c_free(char_content);
    } else fskprint("$3%s$B file not found\n", file);
    c_free(file);
    c_free(fpath);
    c_free(suffix);
    return 0;
}

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; c_str_cpy(result, old);
    if (result[c_str_len(result) - 1] != '/') c_str_append(result, '/');
    for (int i = 0; i < c_str_len(new); i++) c_str_append(result, new[i]);
}
