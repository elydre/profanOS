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

void window_draw(vgui_t *vgui, window_t *window);
void desktop_draw(desktop_t *desktop);
int *sort_index_by_priority(window_t **windows, int nb_windows);
void serial_print_number(char *str, int number);

window_t *window_create(desktop_t* desktop, char *name, int x, int y, int width, int height, int priorite) {
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
    desktop->nb_windows++;
    // on dessine la fenetre
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
    // we add the name of the window
    vgui_print(vgui, window->x + 5, window->y + 5, window->name, 0xffffff);
}

void desktop_refresh(desktop_t *desktop) {
    // we draw the windows, by order of priority
    int total = desktop->nb_windows;

    int *sorted = sort_index_by_priority(desktop->windows, total);

    for (int i = 0; i < total; i++) {
        window_draw(desktop->vgui, desktop->windows[sorted[i]]);
        serial_print_number("drawing window ", sorted[i]);
        c_serial_print(SERIAL_PORT_A, "(");
        c_serial_print(SERIAL_PORT_A, desktop->windows[sorted[i]]->name);
        c_serial_print(SERIAL_PORT_A, ")\n");
    }

    c_serial_print(SERIAL_PORT_A, "FINISHED DRAWING\n");
    vgui_render(desktop->vgui, 0);

    free(sorted);
}

void window_move(window_t *window, int x, int y) {
    window->x = x;
    window->y = y;
    window->has_moved = 1;
}

int *sort_index_by_priority(window_t **windows, int nb_windows) {
    int *sorted = malloc(sizeof(int) * nb_windows);
    
    for (int i = 0; i < nb_windows; i++) {
        sorted[i] = i;
    }

    int tmp = 0;

    for (int i = 0; i < nb_windows; i++) {
        for (int j = 0; j < nb_windows; j++) {
            if (windows[sorted[i]]->priorite < windows[sorted[j]]->priorite) {
                tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }

    return sorted;
}

void serial_print_number(char *str, int number) {
    char *number_str = malloc(sizeof(char) * 10);
    itoa(number, number_str, 10);
    c_serial_print(SERIAL_PORT_A, str);
    c_serial_print(SERIAL_PORT_A, number_str);
    c_serial_print(SERIAL_PORT_A, "\n");
    free(number_str);
}
