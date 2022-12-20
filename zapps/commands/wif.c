#include <syscall.h>
#include <string.h>
#include <iolib.h>
#include <mem.h>

void assemble_path(char old[], char new[], char result[]);

int main(int argc, char** argv) {
    char *current_dir = malloc(256);
    char *suffix = malloc(256);
    str_cpy(current_dir, argv[1]);
    str_cpy(suffix, argv[2]);
    char *file = malloc(str_len(suffix)+str_len(current_dir)+2);
    assemble_path(current_dir, suffix, file);
    if (c_fs_does_path_exists(file) && c_fs_get_sector_type(c_fs_path_to_id(file)) == 2) {
        char char_content[70];
        fsprint("-> "); input(char_content, 70, c_blue); fsprint("\n");
        c_fs_write_in_file(file, (uint8_t *) char_content, str_len(char_content));
    } else fsprint("$3%s$B file not found\n", file);
    free(file);
    free(current_dir);
    free(suffix);
    return 0;
}

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; str_cpy(result, old);
    if (result[str_len(result) - 1] != '/') str_append(result, '/');
    for (int i = 0; i < str_len(new); i++) str_append(result, new[i]);
}
