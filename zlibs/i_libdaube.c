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

window_t *window_create(char *name, int height, int width, int y, int x, int priorite) {
    window_t *window = malloc(sizeof(window_t));
    window->name = malloc(sizeof(char) * strlen(name));
    strcpy(window->name, name);
    window->priorite = priorite;
    window->height = height;
    window->width = width;
    window->y = y;
    window->x = x;
    window->buffer = malloc(sizeof(char) * height * width);
    window->has_moved = 0;
    return window;
}

void window_draw(vgui_t *vgui, window_t *window) {
    // we draw the border of the window
    vgui_draw_line(vgui, window->x, window->y, window->x + window->width, window->y, 0xff0000);
    vgui_draw_line(vgui, window->x, window->y, window->x, window->y + window->height, 0xff0000);
    vgui_draw_line(vgui, window->x + window->width, window->y, window->x + window->width, window->y + window->height, 0xff0000);
    vgui_draw_line(vgui, window->x, window->y + window->height, window->x + window->width, window->y + window->height, 0xff0000);
    // we add the inside (black rectangle)
    vgui_draw_rect(vgui, window->x + 1, window->y + 1, window->width - 1, window->height - 1, 0x000000);
}

void desktop_draw(desktop_t *desktop) {
    // we draw the windows, by order of priority
    int *buffer = calloc(sizeof(int), desktop->vgui->height * desktop->vgui->width);
    int total = desktop->nb_windows;
    int i = desktop->nb_windows - 1;
    while (total) {
        for (int j = 0; j < desktop->nb_windows; j++) {
            if (desktop->windows[j]->priorite == i) {
                window_draw(desktop->vgui, desktop->windows[j]);
                total--;
            }
        }
        i--;
    } 
    vgui_render(desktop->vgui, 0);
    free(buffer);
}

void window_move(desktop_t *desktop, int id_window, int y, int x) {
    desktop->windows[id_window]->y = y;
    desktop->windows[id_window]->x = x;
}
