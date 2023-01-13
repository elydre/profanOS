#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <type.h>
#include <i_vgui.h>
#define LIBDAUBE_C
#include <i_libdaube.h>

void init_func();

int main() {
    init_func();
    return 0;
}

void init_func() {
    printf("Init of the libdaube library\n");
}

window_t *window_create(char *name, int height, int width, int y, int x) {
    window_t *window = malloc(sizeof(window_t));
    window->name = malloc(sizeof(char) * strlen(name));
    strcpy(window->name, name);
    window->height = height;
    window->width = width;
    window->y = y;
    window->x = x;
    window->buffer = malloc(sizeof(char) * height * width);
    return window;
}

void window_draw(vgui_t *vgui, window_t *window) {
    // we draw the border of the window
    vgui_draw_line(vgui, window->x, window->y, window->x + window->width, window->y, 0xff0000);
    vgui_draw_line(vgui, window->x, window->y, window->x, window->y + window->height, 0xff0000);
    vgui_draw_line(vgui, window->x + window->width, window->y, window->x + window->width, window->y + window->height, 0xff0000);
    vgui_draw_line(vgui, window->x, window->y + window->height, window->x + window->width, window->y + window->height, 0xff0000);

}
