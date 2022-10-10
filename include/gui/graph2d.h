#ifndef GRAPH2D_H
#define GRAPH2D_H

typedef struct Sprite_t {
    char *path;;
    char **data;
    char **old_data_buffer;
    int x;
    int y;
    int size_x;
    int size_y;
} Sprite_t;

void lib2d_print_sprite(int x, int y, Sprite_t sprite_path);
void lib2d_free_sprite(Sprite_t sprite);

#endif