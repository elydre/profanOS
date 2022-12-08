#include <klib/filesystem.h>
#include <gui/graph2d.h>
#include <gui/vgui.h>
#include <string.h>
#include <system.h>
#include <mem.h>

sprite_t lib2d_init_sprite(char *path) {
    if (!(fs_does_path_exists(path) && fs_type_sector(fs_path_to_id(path, 0)) == 2)) {
        sys_error("File not found");
        return (sprite_t) {"", NULL, 0, 0, 0, 0};
    }
    uint32_t *file_content = fs_declare_read_array(path);
    char *char_content = fs_declare_read_array(path);
    fs_read_file(path, file_content);
    int char_count;
    int file_size = 0;
    for (char_count = 0; file_content[char_count] != (uint32_t) -1; char_count++) {
        file_size++;
        char_content[char_count] = (char) file_content[char_count];
    }
    free(file_content);
    char_content[char_count] = '\0';
    int index_char = 0;
    char size_hauteur[4];
    int index_hauteur = 0;
    char size_longueur[4];
    int index_longueur = 0;

    while (char_content[index_char] != '|') {
        size_hauteur[index_hauteur] = char_content[index_char];
        index_hauteur++; index_char++;
    }
    size_hauteur[index_hauteur] = '\0';
    index_char++;

    while (char_content[index_char] != '|') {
        size_longueur[index_longueur] = char_content[index_char];
        index_longueur++; index_char++;
    }
    size_longueur[index_longueur] = '\0';
    index_char++;

    int hauteur = ascii_to_int(size_hauteur);
    int longueur = ascii_to_int(size_longueur);
    char *data = calloc(file_size);
    for (int i=0; i<(file_size-index_char); i++) {
        data[i] = char_content[i+index_char];
    }
    free(char_content);

    sprite_t sprite = {path, data, 0, 0, longueur, hauteur};
    return sprite;
}

void lib2d_print_sprite(int x, int y, sprite_t sprite) {
    if (sprite.data == NULL) {
        sys_error("Sprite not initialized");
        return;
    }
    int hauteur = sprite.size_y;
    int longueur = sprite.size_x;

    int index_char = 0;
    char couleur_str[3];
    int index_couleur = 0;
    int couleur = 0;
    for (int h = 0; h<hauteur; h++) {
        for (int l = 0; l<longueur; l++) {
            for (int i = 0; i < 3; i++) couleur_str[i] = '\0';
            index_couleur = 0;
            while (sprite.data[index_char] != '|') {
                couleur_str[index_couleur] = sprite.data[index_char];
                index_couleur++; index_char++;
            }
            couleur_str[index_couleur] = '\0';
            index_char++;
            couleur = ascii_to_int(couleur_str);
            vgui_set_pixel(l+y, h+x, couleur);  // TODO: vgui or vesa
        }
    }
}

void lib2d_free_sprite(sprite_t sprite) {
    free(sprite.data);
}
