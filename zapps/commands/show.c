#include <syscall.h>
#include <string.h>
#include <iolib.h>

void assemble_path(char old[], char new[], char result[]);

int main(int argc, char **argv) {
    char *current_dir = c_malloc(256);
    str_cpy(current_dir, argv[1]);
    char *suffix = c_malloc(256);
    str_cpy(suffix, argv[2]);
    if(!(str_count(suffix, '.'))) str_cat(suffix, ".img");
    char *file = c_malloc(str_len(suffix) + str_len(current_dir) + 2);
    assemble_path(current_dir, suffix, file);

    if (c_fs_does_path_exists(file) && c_fs_type_sector(c_fs_path_to_id(file, 0)) == 2) {
        sprite_t sprite = c_lib2d_init_sprite(file);
        c_lib2d_print_sprite(0, 0, sprite);
        while (c_kb_get_scancode() != 1);
        c_lib2d_free_sprite(sprite);
    } else fsprint("$3%s$B file not found\n", file);
    
    c_free(current_dir);
    c_free(suffix);
    c_free(file);
    return 0;
}

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; str_cpy(result, old);
    if (result[str_len(result) - 1] != '/') str_append(result, '/');
    for (int i = 0; i < str_len(new); i++) str_append(result, new[i]);
}
