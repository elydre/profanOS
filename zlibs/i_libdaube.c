#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <type.h>
#include <i_vgui.h>
#define LIBDAUBE_C
#include <i_libdaube.h>

#define COLOR_MASTER 0x6b6e8c
#define COLOR_TITLES 0xa7a0b9
#define COLOR_GRADN1 0x240865
#define COLOR_GRADN2 0x0c0f1d

void init_func();

int main() {
    init_func();
    return 0;
}

void init_func() {
    printf("Init of the libdaube library\n");
}

void window_draw_box(vgui_t *vgui, window_t *window);
void desktop_draw(desktop_t *desktop);
int *sort_index_by_priority(window_t **windows, int nb_windows);
void serial_print_ss(char *str, char *name);
void draw_rect_gradian(vgui_t *vgui, int x, int y, int width, int height, int color1, int color2);
void window_update_visible(desktop_t *desktop, window_t *window);

window_t *window_create(desktop_t* desktop, char *name, int x, int y, int width, int height, int priorite, int is_lite) {
    serial_print_ss("Creating window", name);
    window_t *window = malloc(sizeof(window_t));
    window->name = malloc(sizeof(char) * strlen(name) + 1);
    strcpy(window->name, name);
    window->priorite = priorite;

    window->in_height = height;
    window->in_width = width;
    window->in_y = y;
    window->in_x = x;

    if (is_lite) {
        window->out_height = height + 2;
        window->out_width = width + 2;
        window->out_y = y - 1;
        window->out_x = x - 1;
    } else {
        window->out_height = height + 26;
        window->out_width = width + 11;
        window->out_y = y - 21;
        window->out_x = x - 6;
    }

    window->buffer = malloc(sizeof(uint32_t) * window->out_height * window->out_width);
    window->visible = malloc(sizeof(uint8_t) * window->out_height * window->out_width);
    window->is_lite = is_lite;

    desktop->nb_windows++;

    return window;
}

void window_draw_box(vgui_t *vgui, window_t *window) {
    // we draw the border of the window
    vgui_draw_line(vgui, window->out_x, window->out_y, window->out_x + window->out_width, window->out_y, COLOR_MASTER);
    vgui_draw_line(vgui, window->out_x, window->out_y, window->out_x, window->out_y + window->out_height, COLOR_MASTER);
    vgui_draw_line(vgui, window->out_x + window->out_width, window->out_y, window->out_x + window->out_width, window->out_y + window->out_height, COLOR_MASTER);
    vgui_draw_line(vgui, window->out_x, window->out_y + window->out_height, window->out_x + window->out_width, window->out_y + window->out_height, COLOR_MASTER);

    // we add the inside
    vgui_draw_rect(vgui, window->out_x + 1, window->out_y + 1, window->out_width - 1, window->out_height - 1, 0x000000);

    if (!window->is_lite) {
        // we add the inside lines
        vgui_draw_line(vgui, window->out_x + 5, window->out_y + 20, window->out_x + window->out_width - 5, window->out_y + 20, COLOR_MASTER);
        vgui_draw_line(vgui, window->out_x + 5, window->out_y + 20, window->out_x + 5, window->out_y + window->out_height - 5, COLOR_MASTER);
        vgui_draw_line(vgui, window->out_x + window->out_width - 5, window->out_y + 20, window->out_x + window->out_width - 5, window->out_y + window->out_height - 5, COLOR_MASTER);
        vgui_draw_line(vgui, window->out_x + 5, window->out_y + window->out_height - 5, window->out_x + window->out_width - 5, window->out_y + window->out_height - 5, COLOR_MASTER);

        // we add the gradian
        draw_rect_gradian(vgui, window->out_x + 5, window->out_y + 1, window->out_width - 9, 18, COLOR_GRADN1, COLOR_GRADN2);
        vgui_draw_rect(vgui, window->out_x + 1, window->out_y + 1, 4, window->out_height - 1, COLOR_GRADN1);
        vgui_draw_rect(vgui, window->out_x + window->out_width - 4, window->out_y + 1, 4, window->out_height - 1, COLOR_GRADN2);
        draw_rect_gradian(vgui, window->out_x + 5, window->out_y + window->out_height - 4, window->out_width - 9, 3, COLOR_GRADN1, COLOR_GRADN2);
    }

    // we add the name of the window
    vgui_print(vgui, window->out_x + 6, window->out_y + 4, window->name, COLOR_TITLES);

    vgui_render(vgui, 0);
}

void desktop_refresh(desktop_t *desktop) {
    // we draw the windows, by order of priority
    int total = desktop->nb_windows;

    int *sorted = sort_index_by_priority(desktop->windows, total);

    for (int i = 0; i < total; i++) {
        serial_print_ss("drawing window box for", desktop->windows[sorted[i]]->name);
        window_draw_box(desktop->vgui, desktop->windows[sorted[i]]);
        serial_print_ss("updating visible of window", desktop->windows[sorted[i]]->name);
        window_update_visible(desktop, desktop->windows[sorted[i]]);
    }

    c_serial_print(SERIAL_PORT_A, "FINISHED DRAWING\n");
    vgui_render(desktop->vgui, 0);

    free(sorted);
}

void window_move(window_t *window, int x, int y) {
    // TODO in_x and in_y
    window->out_x = x;
    window->out_y = y;
}

void window_resize(window_t *window, int width, int height) {
    // TODO
}

void window_fill(window_t *window, uint32_t color) {
    for (int i = 0; i < window->in_width; i++) {
        for (int j = 0; j < window->in_height; j++) {
            window->buffer[i + j * window->in_width] = color;
        }
    }
}

void window_refresh(desktop_t *desktop, window_t *window) {
    serial_print_ss("refreshing window", window->name);
    for (int i = 0; i < window->in_width; i++) {
        for (int j = 0; j < window->in_height; j++) {
            if (window->visible[i + j * window->in_width]) {
                vgui_set_pixel(desktop->vgui, window->in_x + i, window->in_y + j, window->buffer[i + j * window->in_width]);
            }
        }
    }
    vgui_render(desktop->vgui, 0);
}

int *sort_index_by_priority(window_t **windows, int nb_windows) {
    // the first element is the one with the lowest priority

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

void window_update_visible(desktop_t *desktop, window_t *window) {
    // update the visible buffer of the window
    // the visible buffer is a buffer of 1 and 0, 1 if the pixel is visible, 0 if not

    // we first set all the pixels to 1
    for (int i = 0; i < window->out_width * window->out_height; i++) {
        window->visible[i] = 1;
    }

    // we then set the pixels to 0 if they are covered by another window
    int total = desktop->nb_windows;
    int *sorted = sort_index_by_priority(desktop->windows, total);
    // the first element is the one with the lowest priority

    for (int i = total - 1; i >= 0; i--) {
        if (desktop->windows[sorted[i]] == window) {
            break;
        }
        if (desktop->windows[sorted[i]]->visible) {
            for (int j = 0; j < window->in_width; j++) {
                for (int k = 0; k < window->in_height; k++) {
                    if (window->in_x + j >= desktop->windows[sorted[i]]->out_x && window->in_x + j < desktop->windows[sorted[i]]->out_x + desktop->windows[sorted[i]]->out_width && window->in_y + k >= desktop->windows[sorted[i]]->out_y && window->in_y + k < desktop->windows[sorted[i]]->out_y + desktop->windows[sorted[i]]->out_height) {
                        window->visible[j + k * window->in_width] = 0;
                    }
                }
            }
        }
    }

    free(sorted);
}

void serial_print_ss(char *str, char *name) {
    c_serial_print(SERIAL_PORT_A, str);
    c_serial_print(SERIAL_PORT_A, " ");
    c_serial_print(SERIAL_PORT_A, name);
    c_serial_print(SERIAL_PORT_A, "...\n");
}

void draw_rect_gradian(vgui_t *vgui, int x, int y, int width, int height, int color1, int color2) {
    int r1 = (color1 & 0xff0000) >> 16;
    int g1 = (color1 & 0x00ff00) >> 8;
    int b1 = (color1 & 0x0000ff) >> 0;

    float dr = (((color2 & 0xff0000) >> 16) - r1) / (float) width;
    float dg = (((color2 & 0x00ff00) >> 8)  - g1) / (float) width;
    float db = (((color2 & 0x0000ff) >> 0)  - b1) / (float) width;

    float r = r1;
    float g = g1;
    float b = b1;

    int color;

    for (int i = 0; i < width; i++) {
        color = ((int) r << 16) | ((int) g << 8) | (int) b;
        vgui_draw_line(vgui, x + i, y, x + i, y + height, color);
        r += dr;
        g += dg;
        b += db;
    }
}
