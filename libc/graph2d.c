#include <gui/graph2d.h>
#include <filesystem.h>
#include <gui/vga.h>
#include <string.h>
#include <system.h>
#include <iolib.h>
#include <mem.h>

void lib2d_print_sprite(int x, int y, Sprite_t sprite) {
    if (sprite.data != NULL) { // si on a déja la data en cache
        return;
    }
    char *sprite_path = sprite.path;
    if (!(fs_does_path_exists(sprite_path) && fs_type_sector(fs_path_to_id(sprite_path, 0)) == 2)) {
        sys_error("Le fichier demandé dans lib2d_print_sprite n'existe pas !");
        return;
    }
    uint32_t * file_content = fs_declare_read_array(sprite_path);
    char * char_content = fs_declare_read_array(sprite_path);
    fs_read_file(sprite_path, file_content);
    int char_count;
    for (char_count = 0; file_content[char_count] != (uint32_t) -1; char_count++) {
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
    } size_hauteur[index_hauteur] = '\0';
    index_char++;
    while (char_content[index_char] != '|') {
        size_longueur[index_longueur] = char_content[index_char];
        index_longueur++; index_char++;
    } size_longueur[index_longueur] = '\0';
    index_char++;
    int hauteur = ascii_to_int(size_hauteur);
    int longueur = ascii_to_int(size_longueur);

    char couleur_str[3];
    int index_couleur = 0;
    int couleur = 0;
    for (int h = 0; h<hauteur; h++) {
        for (int l = 0; l<longueur; l++) {
            for (int i=0; i<3; i++) {couleur_str[i] = '\0';}
            index_couleur = 0;
            while (char_content[index_char] != '|') {
                couleur_str[index_couleur] = char_content[index_char];
                index_couleur++; index_char++;
            } couleur_str[index_couleur] = '\0'; index_char++;
            couleur = ascii_to_int(couleur_str);
            vga_put_pixel(l+x, h+y, couleur);
        }
    }
    // set nouvelles coordonées
    sprite.x = x;
    sprite.y = y;
    sprite.size_x = longueur;
    sprite.size_y = hauteur;

    // set data
    sprite.data = calloc(hauteur * sizeof(char));
    for (int i = 0; i < sprite.size_y; i++) {
        sprite.data[i] = calloc(longueur * sizeof(char));
    }

    // free
    free(char_content);
}

void lib2d_free_sprite(Sprite_t sprite) {
    for (int i = 0; i < sprite.size_y; i++) {
        free(sprite.data[i]);
    }
    free(sprite.data);
}