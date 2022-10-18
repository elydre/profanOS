#include <syscall.h>

void assemble_path(char old[], char new[], char result[]);

int main(int argc, char **argv) {
    char *current_dir = c_malloc(256);
    c_str_cpy(current_dir, argv[1]);
    char *suffix = c_malloc(256);
    c_str_cpy(suffix, argv[2]);
    if(!(c_str_count(suffix, '.'))) c_str_cat(suffix, ".img");
    char *file = c_malloc(c_str_len(suffix) + c_str_len(current_dir) + 2);
    assemble_path(current_dir, suffix, file);
    c_vga_320_mode();
    Sprite_t sprite = c_lib2d_init_sprite(file);
    c_lib2d_print_sprite(0, 0, sprite);
    while (c_kb_get_scancode() != 1);
    c_lib2d_free_sprite(sprite);
    c_vga_text_mode();
    c_free(file);
    c_free(suffix);
    c_free(current_dir);
    return 0;
}

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; c_str_cpy(result, old);
    if (result[c_str_len(result) - 1] != '/') c_str_append(result, '/');
    for (int i = 0; i < c_str_len(new); i++) c_str_append(result, new[i]);
}