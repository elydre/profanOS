#ifndef GRAPH2D_H
#define GRAPH2D_H

typedef struct sprite_t {
    char *path;;
    char *data;
    int x;
    int y;
    int size_x;
    int size_y;
} sprite_t;

void lib2d_print_sprite(int x, int y, sprite_t sprite_path);
void lib2d_free_sprite(sprite_t sprite);
sprite_t lib2d_init_sprite(char *path);

#endif