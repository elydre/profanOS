#include "syscall.h"

void draw_map(int hauteur, int longueur, char **map);

int main(int argc, char **argv) {
    // read file
    char file_path[] = "/zada/map";
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

    c_vga_320_mode();
    
    draw_map(longueur, hauteur, map);

    Sprite_t perso = c_lib2d_init_sprite("/zada/perso.img");
    int perso_x = 0;
    int perso_y = 10;
    c_lib2d_print_sprite(perso_x, perso_y, perso);

    int last_sc = 0;
    while (last_sc != 1) {
        while (last_sc == c_kb_get_scancode());
        last_sc = c_kb_get_scancode();
        if (last_sc == KB_S && (map[perso_x/10+1][perso_y/10] == '0')) {
            perso_x += 10;
            draw_map(longueur, hauteur, map);
            c_lib2d_print_sprite(perso_x, perso_y, perso);
        }
        if (last_sc == KB_Z && (map[perso_x/10-1][perso_y/10] == '0')) {
            perso_x -= 10;
            draw_map(longueur, hauteur, map);
            c_lib2d_print_sprite(perso_x, perso_y, perso);
        }
        if (last_sc == KB_D && (map[perso_x/10][perso_y/10+1] == '0')) {
            perso_y += 10;
            draw_map(longueur, hauteur, map);
            c_lib2d_print_sprite(perso_x, perso_y, perso);
        }
        if (last_sc == KB_Q && (map[perso_x/10][perso_y/10-1] == '0')) {
            perso_y -= 10;
            draw_map(longueur, hauteur, map);
            c_lib2d_print_sprite(perso_x, perso_y, perso);
        }
        if (last_sc == KB_D && (map[perso_x/10][perso_y/10+1] == '2')) {
            perso_y += 10;
            draw_map(longueur, hauteur, map);
            c_lib2d_print_sprite(perso_x, perso_y, perso);
            for (int i=0; i < 10; i++) {
                for (int j=0; j<10; j++) {
                    c_vga_print(25*i, 25*j, "GG !", 1, 36);
                }
            }
            c_ms_sleep(1000);
            break;
        }
    }

    c_vga_text_mode();

    // free
    c_lib2d_free_sprite(perso);
    for (int i = 0; i < hauteur; i++) {
        c_free(map[i]);
    }
    c_free(map);
    c_free(char_content);
    return 0;
}

void draw_map(int hauteur, int longueur, char **map) {
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j < longueur; j++) {
            if (map[i][j] == '0') {
                c_vga_draw_rect(j*10, i*10, 10, 10, 63);
            } else if (map[i][j] == '1') {
                c_vga_draw_rect(j*10, i*10, 10, 10, 0);
            } else if (map[i][j] == '2') {
                c_vga_draw_rect(j*10, i*10, 10, 10, 2);
            } else {
                // unknown
            }
        }
    }
}