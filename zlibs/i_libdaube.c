#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <type.h>

#include <i_time.h>
#include <i_vgui.h>

#define LIBDAUBE_C
#include <i_libdaube.h>

#define COLOR_MASTER 0x6b6e8c
#define COLOR_TITLES 0xa7a0b9
#define COLOR_GRADN1 0x240865
#define COLOR_GRADN2 0x0c0f1d

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

uint8_t mouse_img[21*12] = {
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0,
    1, 2, 2, 1, 1, 2, 2, 1, 0, 0, 0, 0,
    1, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 0,
    1, 1, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
};

void init_func();

int main() {
    init_func();
    return 0;
}

void init_func() {
    printf("Init of the libdaube library\n");
}


void window_draw_box(desktop_t *desktop, window_t *window);
void desktop_draw(desktop_t *desktop);
int *sort_index_by_priority(window_t **windows, int nb_windows);
void serial_print_ss(char *str, char *name);
void window_set_pixels_visible(desktop_t *desktop, window_t *window, int all);
void window_update_visible(desktop_t *desktop, window_t *window);
void set_window_priority(desktop_t *desktop, window_t *window);
mouse_t* mouse_create();

void draw_rect_gradian(window_t *window, int x, int y, int width, int height, int color1, int color2);
void draw_straight_line(window_t *window, int x1, int y1, int x2, int y2, int color);
void draw_rect(window_t *window, int x, int y, int width, int height, int color);
void draw_print_wut(window_t *window, int x, int y, char *msg, int color);

desktop_t *desktop_init(vgui_t *vgui, int max_windows, int screen_width, int screen_height) {
    desktop_t *desktop = malloc(sizeof(desktop_t));
    desktop->nb_windows = 0;
    desktop->vgui = vgui;
    desktop->windows = malloc(sizeof(window_t *) * max_windows);
    desktop->mouse = mouse_create();

    desktop->screen_width = screen_width;
    desktop->screen_height = screen_height;

    return desktop;
}

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
        window->out_height = height + 27;
        window->out_width = width + 12;
        window->out_y = y - 21;
        window->out_x = x - 6;
    }

    window->parent_desktop = (void *) desktop;
    window->buffer = calloc(window->out_height * window->out_width, sizeof(uint32_t));
    window->visible = malloc(sizeof(uint8_t) * window->out_height * window->out_width);
    window->is_lite = is_lite;
    window->changed = 1;

    desktop->nb_windows++;

    window_draw_box(desktop, window);

    return window;
}

void window_draw_box(desktop_t *desktop, window_t *window) {
    // we draw the border of the window
    draw_straight_line(window, 0, 0, window->out_width - 1, 0, COLOR_MASTER);
    draw_straight_line(window, 0, 0, 0, window->out_height - 1, COLOR_MASTER);
    draw_straight_line(window, window->out_width - 1, 0, window->out_width - 1, window->out_height, COLOR_MASTER);
    draw_straight_line(window, 0, window->out_height - 1, window->out_width - 1, window->out_height - 1, COLOR_MASTER);

    if (!window->is_lite) {
        // we add the inside lines
        draw_straight_line(window, 5, 20, window->out_width - 6, 20, COLOR_MASTER);
        draw_straight_line(window, 5, 20, 5, window->out_height - 6, COLOR_MASTER);
        draw_straight_line(window, window->out_width - 6, 20, window->out_width - 6, window->out_height - 5, COLOR_MASTER);
        draw_straight_line(window, 5, window->out_height - 6, window->out_width - 6, window->out_height - 6, COLOR_MASTER);

        // we add the gradians
        draw_rect_gradian(window, 5, 1, window->out_width - 9, 19, COLOR_GRADN1, COLOR_GRADN2);
        draw_rect_gradian(window, 5, window->out_height - 5, window->out_width - 9, 4, COLOR_GRADN1, COLOR_GRADN2);

        // and the rectangles
        draw_rect(window, 1, 1, 4, window->out_height - 2, COLOR_GRADN1);
        draw_rect(window, window->out_width - 5, 1, 4, window->out_height - 2, COLOR_GRADN2);

        // we add the name of the window
        draw_print_wut(window, 6, 4, window->name, COLOR_TITLES);
    }
}

void desktop_refresh(desktop_t *desktop) {
    desktop->is_locked = 1;

    // we draw the windows, by order of priority
    int total = desktop->nb_windows;
    int *sorted = sort_index_by_priority(desktop->windows, total);

    for (int i = 0; i < total; i++) {
        serial_print_ss("work for", desktop->windows[sorted[i]]->name);
        window_update_visible(desktop, desktop->windows[sorted[i]]);
        window_set_pixels_visible(desktop, desktop->windows[sorted[i]], 0);
    }

    for (int i = 0; i < total; i++) {
        desktop->windows[i]->changed = 0;
    }

    c_serial_print(SERIAL_PORT_A, "FINISHED DRAWING\n");
    vgui_render(desktop->vgui, 0);

    desktop->is_locked = 0;

    free(sorted);
}

void window_move(window_t *window, int x, int y) {
    if (window->is_lite) {
        window->out_y = y - 1;
        window->out_x = x - 1;
    } else {
        window->out_y = y - 21;
        window->out_x = x - 6;
    }
    window->in_x = x;
    window->in_y = y;

    window->changed = 1;
}

void window_resize(window_t *window, int width, int height) {
    // TODO
}

void window_set_pixel(window_t *window, int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= window->in_width || y >= window->in_height) {
        return;
    }

    x += window->in_x - window->out_x;
    y += window->in_y - window->out_y;

    window->buffer[y * window->out_width + x] = color;
}

void window_fill(window_t *window, uint32_t color) {
    for (int i = 0; i < window->in_width; i++) {
        for (int j = 0; j < window->in_height; j++) {
            window_set_pixel(window, i, j, color);
        }
    }
}

void window_refresh(window_t *window) {
    desktop_t *desktop = window->parent_desktop;

    while (desktop->is_locked) {
        // serial_print_ss("desktop is locked, can't refresh", window->name);
        ms_sleep(1);
    }

    window_set_pixels_visible(desktop, window, 1);
    vgui_render(desktop->vgui, 0);
}

mouse_t* mouse_create() {
    mouse_t *mouse = malloc(sizeof(mouse_t));
    mouse->x = 0;
    mouse->y = 0;
    mouse->size_x = 12;
    mouse->size_y = 21;
    return mouse;
}

void refresh_mouse(desktop_t *desktop) {
    // restore the image under the mouse
    for (int i = 0; i < desktop->mouse->size_x; i++) {
        for (int j = 0; j < desktop->mouse->size_y; j++) {
            c_vesa_set_pixel(desktop->mouse->x + i, desktop->mouse->y + j, vgui_get_pixel(desktop->vgui, desktop->mouse->x + i, desktop->mouse->y + j));
        }
    }

    desktop->mouse->x = c_mouse_call(0, 0);
    desktop->mouse->y = c_mouse_call(1, 0);

    for (int i = 0; i < desktop->mouse->size_x; i++) {
        for (int j = 0; j < desktop->mouse->size_y; j++) {
            if (mouse_img[i + j * desktop->mouse->size_x] == 1) {
                c_vesa_set_pixel(desktop->mouse->x + i, desktop->mouse->y + j, COLOR_MASTER);
            } else if (mouse_img[i + j * desktop->mouse->size_x] == 2) {
                c_vesa_set_pixel(desktop->mouse->x + i, desktop->mouse->y + j, COLOR_GRADN2);
            }
        }
    }

    int is_clicked = c_mouse_call(2, 0);
    if (is_clicked && !desktop->mouse->already_clicked) {
        serial_print_ss("mouse", "clicked");
        // we check if the mouse is on a window
        int total = desktop->nb_windows;
        int *sorted = sort_index_by_priority(desktop->windows, desktop->nb_windows);
        // the first element is the one with the lowest priority
        for (int i = total - 1; i >= 0; i--) {
            window_t *window = desktop->windows[sorted[i]];
            if (desktop->mouse->x >= window->out_x && desktop->mouse->x <= window->out_x + window->out_width && desktop->mouse->y >= window->out_y && desktop->mouse->y <= window->out_y + window->out_height) {
                set_window_priority(desktop, window);
                window->changed = 1;
                desktop_refresh(desktop);
                break;
            }
        }
    }

    if (is_clicked) {
        desktop->mouse->already_clicked = 1;
    } else {
        desktop->mouse->already_clicked = 0;
    }

    if (c_mouse_call(2, 1)) {
        desktop_refresh(desktop);
    }
}


void window_set_pixels_visible(desktop_t *desktop, window_t *window, int all) {
    if (window->changed || all) {
        for (int i = 0; i < window->out_width; i++) {
            for (int j = 0; j < window->out_height; j++) {
                if (window->visible[i + j * window->out_width]) {
                    vgui_set_pixel(desktop->vgui, window->out_x + i, window->out_y + j, window->buffer[i + j * window->out_width]);
                }
            }
        }
        return;
    }

    int total = desktop->nb_windows;
    int *sorted = sort_index_by_priority(desktop->windows, desktop->nb_windows);
    // the first element is the one with the lowest priority
    
    int x1, x2, y1, y2;

    for (int i = 0; i < total; i++) {
        window_t *w = desktop->windows[sorted[i]];
        if (w->priorite >= window->priorite) break;
        if (! w->changed) continue;
        // calculate the intersection of the two rectangles
        x1 = max(window->out_x, w->out_x);
        x2 = min(window->out_x + window->out_width, w->out_x + w->out_width);
        y1 = max(window->out_y, w->out_y);
        y2 = min(window->out_y + window->out_height, w->out_y + w->out_height);

        for (int x = x1; x < x2; x++) {
            for (int y = y1; y < y2; y++) {
                if (window->visible[(x - window->out_x) + (y - window->out_y) * window->out_width]) {
                    vgui_set_pixel(desktop->vgui, x, y, window->buffer[(x - window->out_x) + (y - window->out_y) * window->out_width]);
                }
            }
        }
    }
    free(sorted);
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

void set_window_priority(desktop_t *desktop, window_t *window) {
    int total = desktop->nb_windows;
    int *sorted = sort_index_by_priority(desktop->windows, total);
    int current_priority = window->priorite;
    // the first element is the one with the lowest priority

    for (int i = total - 1; i >= current_priority; i--) {
        if (desktop->windows[sorted[i]] == window) {
            window->priorite = total - 1;
        } else {
            desktop->windows[sorted[i]]->priorite--;
            desktop->windows[sorted[i]]->changed = 1;
        }
    }
}

void window_update_visible(desktop_t *desktop, window_t *window) {
    // can be optimized...

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

    int x1, x2, y1, y2;

    for (int i = total - 1; i >= 0; i--) {
        if (desktop->windows[sorted[i]] == window) {
            break;
        }

        // calculate the intersection of the two rectangles
        x1 = max(window->out_x, desktop->windows[sorted[i]]->out_x);
        x2 = min(window->out_x + window->out_width, desktop->windows[sorted[i]]->out_x + desktop->windows[sorted[i]]->out_width);
        y1 = max(window->out_y, desktop->windows[sorted[i]]->out_y);
        y2 = min(window->out_y + window->out_height, desktop->windows[sorted[i]]->out_y + desktop->windows[sorted[i]]->out_height);

        for (int x = x1; x < x2; x++) {
            for (int y = y1; y < y2; y++) {
                window->visible[x - window->out_x + (y - window->out_y) * window->out_width] = 0;
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

void draw_straight_line(window_t *window, int x1, int y1, int x2, int y2, int color) {
    // just orizontal or vertical lines
    if (x1 == x2) {
        for (int i = y1; i < y2; i++) {
            window->buffer[x1 + i * window->out_width] = color;
        }
    } else if (y1 == y2) {
        for (int i = x1; i < x2; i++) {
            window->buffer[i + y1 * window->out_width] = color;
        }
    }
}

void draw_rect_gradian(window_t *window, int x, int y, int width, int height, int color1, int color2) {
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
        draw_straight_line(window, x + i, y, x + i, y + height, color);
        r += dr;
        g += dg;
        b += db;
    }
}

void draw_rect(window_t *window, int x, int y, int width, int height, int color) {
    for (int i = x; i < x + width; i++) {
        for (int j = y; j < y + height; j++) {
            window->buffer[i + j * window->out_width] = color;
        }
    }
}

void draw_print_wut(window_t *window, int x, int y, char *msg, int color) {
    uint8_t *glyph;
    for (int i = 0; msg[i] != '\0'; i++) {
        glyph = c_font_get(0) + msg[i] * 16;
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 8; k++) {
                if (!(glyph[j] & (1 << k))) continue;
                window->buffer[(i * 8 + x + 8 - k) + (y + j) * window->out_width] = color;
            }
        }
    }
}
