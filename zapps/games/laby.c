#include "../include/driver/keyboard.h"
#include "syscall.h"

int main(int arg) {
    
    // read file
    char file_path[] = "/user/map";
    if (!(c_fs_does_path_exists(file_path) && c_fs_type_sector(c_fs_path_to_id(file_path, 0)) == 2)) {
        c_fskprint("File not found\n");
        return 0;
    }
    uint32_t * file_content = c_fs_declare_read_array(file_path);
    char * char_content = c_fs_declare_read_array(file_path);
    c_fs_read_file(file_path, file_content);
    int char_count;
    for (char_count = 0; file_content[char_count] != (uint32_t) -1; char_count++)
        char_content[char_count] = (char) file_content[char_count];
    char_content[char_count] = '\0';
    c_free(file_content);
    
    // get hauteur et longueur
    int index_char = 0;
    char size_hauteur[4];
    int index_hauteur = 0;
    char size_longueur[4];
    int index_longueur = 0;
    while (char_content[index_char] != '|') {
        size_hauteur[index_hauteur] = char_content[index_char];
        index_hauteur++; index_char++;
    } size_hauteur[index_hauteur] = '\0';
    index_char++;
    while (char_content[index_char] != '|') {
        size_longueur[index_longueur] = char_content[index_char];
        index_longueur++; index_char++;
    } size_longueur[index_longueur] = '\0';
    index_char++;
    int hauteur = c_ascii_to_int(size_hauteur);
    int longueur = c_ascii_to_int(size_longueur);

    c_fskprint("hauteur: %d, longueur: %d\n", hauteur, longueur);
    c_str_end_split(char_content, '|');
    c_str_end_split(char_content, '|');
    char **map = c_calloc(hauteur*sizeof(char *));
    for (int i = 0; i < hauteur; i++) {
        map[i] = c_calloc(longueur*sizeof(char));
    }
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j < longueur; j++) {
            map[i][j] = char_content[i * longueur + j];
        }
    }
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j < longueur; j++) {
            c_fskprint("%c", map[i][j]);
        }
        c_fskprint("\n");
    }

    c_vga_320_mode();

    // draw map
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j < longueur; j++) {
            if (map[i][j] == '1') {
                c_vga_draw_rect(j*10, i*10, 10, 10, 0);
            } else {
                c_vga_draw_rect(j*10, i*10, 10, 10, 63);
            }
        }
    }

    char *sprite_path = "/zada/perso.img";
    c_lib2d_print_sprite(0, 0, sprite_path);

    while(1) {
        int scancode = c_kb_get_scancode();
        if (scancode == KB_S) {
            
        }
    }
    
    c_vga_text_mode();

    // free
    for (int i = 0; i < hauteur; i++) {
        c_free(map[i]);
    }
    c_free(map);
    c_free(char_content);
    return arg;
}