#include <syscall.h>
#include <string.h>
#include <iolib.h>

void assemble_path(char old[], char new[], char result[]);

int main(int argc, char** argv) {
    char *current_dir = c_malloc(256);
    char *suffix = c_malloc(256);
    str_cpy(current_dir, argv[1]);
    str_cpy(suffix, argv[2]);
    char *file = c_malloc(str_len(suffix)+str_len(current_dir)+2);
    assemble_path(current_dir, suffix, file);
    if (c_fs_does_path_exists(file) && c_fs_type_sector(c_fs_path_to_id(file, 0)) == 2) {
        char char_content[70];
        fskprint("-> "); input(char_content, 70, c_blue); fskprint("\n");
        uint32_t *file_content = c_malloc(str_len(char_content));
        for (int i = 0; i < 70; i++) file_content[i] = (uint32_t) char_content[i];
        c_fs_write_in_file(file, file_content, str_len(char_content));
        c_free(file_content);
    } else fskprint("$3%s$B file not found\n", file);
    c_free(file);
    c_free(current_dir);
    c_free(suffix);
    return 0;
}

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; str_cpy(result, old);
    if (result[str_len(result) - 1] != '/') str_append(result, '/');
    for (int i = 0; i < str_len(new); i++) str_append(result, new[i]);
}
