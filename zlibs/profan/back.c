#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <type.h>

#include <i_time.h>
#include <i_vgui.h>

#define LIBDAUBE_C
#include <i_libdaube.h>

#define NO_OPTI 0   // disable optimizations

#define DEBUG_LEVEL 0

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

desktop_t *main_desktop;

#define MOUSE_WIDTH 12
#define MOUSE_HEIGHT 21

void init_func();

int main() {
    init_func();
    return 0;
}

void init_func() {
    main_desktop = NULL;
    printf("Init of the libdaube library\n");
}

window_t *window_create(desktop_t* desktop, char *name, int x, int y, int width, int height, int is_lite, int cant_move);

void window_set_pixels_visible(desktop_t *desktop, window_t *window, int all);
void window_update_visible(desktop_t *desktop, window_t *window);
int *sort_index_by_priority(window_t **windows, int nb_windows);
void set_window_priority(desktop_t *desktop, window_t *window);
void window_draw_box(desktop_t *desktop, window_t *window);
void serial_print_ss(char *str, char *name);
void desktop_draw(desktop_t *desktop);
mouse_t* mouse_create();

void draw_rect_gradian(window_t *window, int x, int y, int width, int height, int color1, int color2);
void draw_straight_line(window_t *window, int x1, int y1, int x2, int y2, int color);
void draw_rect(window_t *window, int x, int y, int width, int height, int color);
void draw_print_wut(window_t *window, int x, int y, char *msg, int color);

desktop_t *desktop_init(vgui_t *vgui, int max_windows, int screen_width, int screen_height) {
    desktop_t *desktop = calloc(1, sizeof(desktop_t));
    desktop->nb_windows = 0;
    desktop->vgui = vgui;
    desktop->windows = calloc(max_windows, sizeof(window_t *));
    desktop->mouse = mouse_create();

    desktop->screen_width = screen_width;
    desktop->screen_height = screen_height;
    desktop->max_windows = max_windows;

    if (main_desktop == NULL) {
        main_desktop = desktop;
    }

    window_create(desktop, "desktop", 1, 1, 1022, 766, 1, 1);

    return desktop;
}

window_t *window_create(desktop_t* desktop, char *name, int x, int y, int width, int height, int is_lite, int cant_move) {
    if (DEBUG_LEVEL > 0) serial_print_ss("Creating window", name);
    if (desktop->nb_windows >= desktop->max_windows) {
        serial_print_ss("Error:", "Too many windows\n");
        return NULL;
    }
    if (width < 0 || height < 0) {
        serial_print_ss("Error:", "Window position is negative\n");
        return NULL;
    }
    window_t *window = calloc(1, sizeof(window_t));
    window->name = calloc(strlen(name) + 1, sizeof(char));
    strcpy(window->name, name);
    window->priority = desktop->nb_windows;

    window->in_height = height;
    window->in_width = width;
    window->in_y = y;
    window->in_x = x;

    if (is_lite) {
        window->height = height + 2;
        window->width = width + 2;
        window->y = y - 1;
        window->x = x - 1;
    } else {
        window->height = height + 27;
        window->width = width + 12;
        window->y = y - 21;
        window->x = x - 6;
    }

    window->parent_desktop = (void *) desktop;
    window->button_array = NULL;
    window->buffer = calloc(window->height * window->width, sizeof(uint32_t));
    window->visible = calloc(window->height * window->width, sizeof(uint8_t));
    window->is_lite = is_lite;
    window->changed = 1;

    window->cant_move = cant_move;

    // draw the border of the window in the buffer
    window_draw_box(desktop, window);

    desktop->nb_windows++;

    // add the window to the desktop
    for (int i = 0; i < desktop->max_windows; i++) {
        if (desktop->windows[i] == NULL) {
            desktop->windows[i] = window;
            break;
        }
    }

    return window;
}

void window_draw_box(desktop_t *desktop, window_t *window) {
    // we draw the border of the window
    draw_straight_line(window, 0, 0, window->width - 1, 0, COLOR_MASTER);
    draw_straight_line(window, 0, 0, 0, window->height - 1, COLOR_MASTER);
    draw_straight_line(window, window->width - 1, 0, window->width - 1, window->height, COLOR_MASTER);
    draw_straight_line(window, 0, window->height - 1, window->width - 1, window->height - 1, COLOR_MASTER);

    if (!window->is_lite) {
        // we add the inside lines
        draw_straight_line(window, 5, 20, window->width - 6, 20, COLOR_MASTER);
        draw_straight_line(window, 5, 20, 5, window->height - 6, COLOR_MASTER);
        draw_straight_line(window, window->width - 6, 20, window->width - 6, window->height - 5, COLOR_MASTER);
        draw_straight_line(window, 5, window->height - 6, window->width - 6, window->height - 6, COLOR_MASTER);

        // we add the gradians
        draw_rect_gradian(window, 5, 1, window->width - 9, 19, COLOR_GRADN1, COLOR_GRADN2);
        draw_rect_gradian(window, 5, window->height - 5, window->width - 9, 4, COLOR_GRADN1, COLOR_GRADN2);

        // and the rectangles
        draw_rect(window, 1, 1, 4, window->height - 2, COLOR_GRADN1);
        draw_rect(window, window->width - 5, 1, 4, window->height - 2, COLOR_GRADN2);

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
        if (DEBUG_LEVEL > 2) serial_print_ss("work for", desktop->windows[sorted[i]]->name);
        window_update_visible(desktop, desktop->windows[sorted[i]]);
        window_set_pixels_visible(desktop, desktop->windows[sorted[i]], 0);
    }

    for (int i = 0; i < total; i++) {
        desktop->windows[i]->changed = 0;
    }

    if (DEBUG_LEVEL > 2) c_serial_print(SERIAL_PORT_A, "FINISHED DRAWING\n");
    vgui_render(desktop->vgui, 0);

    desktop->is_locked = 0;

    free(sorted);
}

void window_move(window_t *window, int x, int y) {
    window->old_x = window->x;
    window->old_y = window->y;

    if (window->is_lite) {
        window->y = y - 1;
        window->x = x - 1;
    } else {
        window->y = y - 21;
        window->x = x - 6;
    }
    window->in_x = x;
    window->in_y = y;

    window->changed = 1;
}

void window_resize(window_t *window, int width, int height) {
    // TODO
}

void window_set_pixel_func(window_t *window, int x, int y, uint32_t color, uint8_t in_box) {
    // the coordinates are those of the interior of the window
    int out_x = x;
    int out_y = y;

    if (in_box) {
        out_x += window->in_x - window->x;
        out_y += window->in_y - window->y;
    }

    if (out_x < 0 || out_x >= window->width || out_y < 0 || out_y >= window->height) return;

    window->buffer[out_y * window->width + out_x] = color;
}

void window_fill(window_t *window, uint32_t color) {
    for (int i = 0; i < window->in_width; i++) {
        for (int j = 0; j < window->in_height; j++) {
            window_set_pixel_func(window, i, j, color, 1);
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
    mouse_t *mouse = calloc(1, sizeof(mouse_t));
    mouse->x = 0;
    mouse->y = 0;
    mouse->clicked_window_id = -1;
    return mouse;
}

void refresh_mouse(desktop_t *desktop) {
    if (desktop->mouse->clicked_window_id != -1) {
        if (desktop->windows[desktop->mouse->clicked_window_id]->cant_move) {
            if (DEBUG_LEVEL > 2) serial_print_ss("window", "cant move");
            desktop->mouse->clicked_window_id = -1;
            desktop->mouse->already_clicked = !desktop->mouse->already_clicked;
            return;
        }
    }

    // restore the image under the mouse
    for (int i = 0; i < MOUSE_WIDTH; i++) {
        for (int j = 0; j < MOUSE_HEIGHT; j++) {
            // chek if we are outside the screen
            if (desktop->mouse->x + i < 0 || desktop->mouse->x + i >= desktop->vgui->width || desktop->mouse->y + j < 0 || desktop->mouse->y + j >= desktop->vgui->height) {
                continue;
            }
            c_vesa_set_pixel(desktop->mouse->x + i, desktop->mouse->y + j, vgui_get_pixel(desktop->vgui, desktop->mouse->x + i, desktop->mouse->y + j));
        }
    }

    if (desktop->mouse->clicked_window_id != -1) {
        window_t *window = desktop->windows[desktop->mouse->clicked_window_id];

        int window_width = window->width;
        int window_height = window->height;

        // draw window outline
        int x = desktop->mouse->x + desktop->mouse->window_x_dec;
        int y = desktop->mouse->y + desktop->mouse->window_y_dec;

        for (int i = 0; i < window_width; i++) {
            // chek if we are outside the screen
            if (x + i - 6 < 0 || x + i - 6 >= desktop->vgui->width) {
                continue;
            }
            c_vesa_set_pixel(x + i - 6, y - 21, vgui_get_pixel(desktop->vgui, x + i - 6, y - 21));
        }
        for (int i = 0; i < window_width; i++) {
            // chek if we are outside the screen
            if (x + i - 6 < 0 || x + i - 6 >= desktop->vgui->width) {
                continue;
            }
            c_vesa_set_pixel(x + i - 6, y + window_height - 22, vgui_get_pixel(desktop->vgui, x + i - 6, y + window_height - 22));
        }
        for (int i = 0; i < window_height; i++) {
            // chek if we are outside the screen
            if (x - 6 < 0 || x - 6 >= desktop->vgui->width) {
                continue;
            }
            c_vesa_set_pixel(x - 6, y + i - 21, vgui_get_pixel(desktop->vgui, x - 6, y + i - 21));
        }
        for (int i = 0; i < window_height; i++) {
            // chek if we are outside the screen
            if (x + window_width - 7 < 0 || x + window_width - 7 >= desktop->vgui->width) {
                continue;
            }
            c_vesa_set_pixel(x + window_width - 7, y + i - 21, vgui_get_pixel(desktop->vgui, x + window_width - 7, y + i - 21));
        }
    }

    desktop->mouse->x = c_mouse_call(0, 0);
    desktop->mouse->y = c_mouse_call(1, 0);

    for (int i = 0; i < MOUSE_WIDTH; i++) {
        for (int j = 0; j < MOUSE_HEIGHT; j++) {
            if (desktop->mouse->x + i < 0 || desktop->mouse->x + i >= desktop->vgui->width || desktop->mouse->y + j < 0 || desktop->mouse->y + j >= desktop->vgui->height) {
                continue;
            }
            if (mouse_img[i + j * MOUSE_WIDTH] == 1) {
                c_vesa_set_pixel(desktop->mouse->x + i, desktop->mouse->y + j, COLOR_MASTER);
            } else if (mouse_img[i + j * MOUSE_WIDTH] == 2) {
                c_vesa_set_pixel(desktop->mouse->x + i, desktop->mouse->y + j, COLOR_GRADN2);
            }
        }
    }

    int is_clicked = c_mouse_call(2, 0);
    if (is_clicked && !desktop->mouse->already_clicked) {
        if (DEBUG_LEVEL > 2) serial_print_ss("mouse", "clicked");
        // we check if the mouse is on a window
        int total = desktop->nb_windows;
        int *sorted = sort_index_by_priority(desktop->windows, desktop->nb_windows);
        int has_clicked = 0;
        // the first element is the one with the lowest priority
        for (int i = total - 1; i >= 0; i--) {
            window_t *window = desktop->windows[sorted[i]];
            if (desktop->mouse->x >= window->x && desktop->mouse->x <= window->x + window->width && desktop->mouse->y >= window->y && desktop->mouse->y <= window->y + window->height) {
                // click de bouton a check
                if (DEBUG_LEVEL > 2) serial_print_ss("mouse", "clicked on window inside");
                for (int j = 0; j < window->buttons_count; j++) {
                    button_t *button = window->button_array[j];
                    if (desktop->mouse->x - window->x >= button->x && desktop->mouse->x - window->x <= button->x + button->width && desktop->mouse->y - window->y >= button->y && desktop->mouse->y - window->y <= button->y + button->height) {
                        button->is_clicked = 1;
                        if (DEBUG_LEVEL > 0) serial_print_ss("button", "clicked");
                        has_clicked = 1;
                        break;
                    }
                }
                if (has_clicked) break;
            }
            if (desktop->mouse->x >= window->x && desktop->mouse->x <= window->x + window->width && desktop->mouse->y >= window->y && desktop->mouse->y <= window->y + 20) {
                if (window->is_lite) {
                    if (DEBUG_LEVEL > 2) serial_print_ss("window cant move", "lite");
                    break;
                }
                if (DEBUG_LEVEL > 2) serial_print_ss("mouse", "clicked on window top");
                is_clicked = 2;
                desktop->mouse->clicked_window_id = sorted[i];
                desktop->mouse->window_x_dec = window->in_x - desktop->mouse->x;
                desktop->mouse->window_y_dec = window->in_y - desktop->mouse->y;
                break;
            }
        }
        is_clicked--;
        free(sorted);
    } else if (is_clicked && desktop->mouse->already_clicked) {
        if (DEBUG_LEVEL > 2) serial_print_ss("mouse", "dragging");
        window_t *window = desktop->windows[desktop->mouse->clicked_window_id];

        int window_width = window->width;
        int window_height = window->height;

        // draw window outline
        int x = desktop->mouse->x + desktop->mouse->window_x_dec;
        int y = desktop->mouse->y + desktop->mouse->window_y_dec;

        for (int i = 0; i < window_width; i++) {
            // chek if we are outside the screen
            if (x + i - 6 < 0 || x + i - 6 >= desktop->vgui->width) {
                continue;
            }
            c_vesa_set_pixel(x + i - 6, y - 21, COLOR_MASTER);
        }
        for (int i = 0; i < window_width; i++) {
            // chek if we are outside the screen
            if (x + i - 6 < 0 || x + i - 6 >= desktop->vgui->width) {
                continue;
            }
            c_vesa_set_pixel(x + i - 6, y + window_height - 22, COLOR_MASTER);
        }
        for (int i = 0; i < window_height; i++) {
            // chek if we are outside the screen
            if (x - 6 < 0 || x - 6 >= desktop->vgui->width) {
                continue;
            }
            c_vesa_set_pixel(x - 6, y + i - 21, COLOR_MASTER);
        }
        for (int i = 0; i < window_height; i++) {
            // chek if we are outside the screen
            if (x + window_width - 7 < 0 || x + window_width - 7 >= desktop->vgui->width) {
                continue;
            }
            c_vesa_set_pixel(x + window_width - 7, y + i - 21, COLOR_MASTER);
        }
    } else if (!is_clicked && desktop->mouse->already_clicked) {
        if (DEBUG_LEVEL > 2) serial_print_ss("mouse", "released");
        if (desktop->mouse->clicked_window_id == -1) {
            serial_print_ss("BUG", "clicked_window_id == -1");
        }
        window_t *window = desktop->windows[desktop->mouse->clicked_window_id];
        set_window_priority(desktop, window);
        window_move(window, desktop->mouse->x + desktop->mouse->window_x_dec, desktop->mouse->y + desktop->mouse->window_y_dec);
        desktop->mouse->clicked_window_id = -1;
        desktop_refresh(desktop);
    }

    if (is_clicked) {
        desktop->mouse->already_clicked = 1;
    } else {
        desktop->mouse->already_clicked = 0;
    }

    if (c_mouse_call(2, 1)) {
        desktop_refresh(desktop);
    }

    for (int i = 0; i < desktop->nb_windows; i++) {
        window_t *window = desktop->windows[i];
        for (int j = 0; j < window->buttons_count; j++) {
            button_t *button = ((button_t **) window->button_array)[j];
            if (button->is_clicked) {
                clickevent_t *event = malloc(sizeof(clickevent_t));
                event->x = desktop->mouse->x - window->x;
                event->y = desktop->mouse->y - window->y;
                event->button = button;
                event->mouse = desktop->mouse;
                button->callback(event);
                free(event);
                button->is_clicked = 0;
            }
        }
    }
}

desktop_t *desktop_get_main() {
    return main_desktop;
}

void window_delete(window_t *window) {
    serial_print_ss("window", "delete");
    // we need to free the memory and switch the priority of every window

    // we need to refresh the desktop
    desktop_t *desktop = window->parent_desktop;
    window->priority = -1;
    window->changed = 1;
    
    desktop_refresh(window->parent_desktop);

    // free the buttons
    for (int i = 0; i < window->buttons_count; i++) {
        free(window->button_array[i]);
    }

    // free the arrays
    free(window->button_array);
    free(window->name);
    free(window->buffer);
    free(window->visible);

    // remove the window
    for (int i = 0; i < desktop->nb_windows; i++) {
        if (desktop->windows[i] == window) {
            desktop->windows[i] = desktop->windows[desktop->nb_windows - 1];
            break;
        }
    }

    desktop->nb_windows--;

    free(window);
}

button_t *create_button(window_t *window, int x, int y, int width, int height, void (*callback)(clickevent_t *)) {
    // set button
    button_t *button = malloc(sizeof(button_t));
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->callback = callback;
    button->window = window;
    button->is_clicked = 0;
    // set in the window
    window->button_array = realloc(window->button_array, sizeof(button_t *) * (window->buttons_count + 1));
    window->button_array[window->buttons_count] = button;
    window->buttons_count++;
    return button;
}

void window_set_pixels_intersection(desktop_t *desktop, window_t *window, window_t *w, uint8_t is_first) {
    int x1, x2, y1, y2;

    if (w->changed && is_first) {
        if (DEBUG_LEVEL > 2) serial_print_ss("window move:", w->name);
        x1 = max(window->x, w->old_x);
        x2 = min(window->x + window->width, w->old_x + w->width);
        y1 = max(window->y, w->old_y);
        y2 = min(window->y + window->height, w->old_y + w->height);
    } else {
        // calculate the intersection of the two rectangles
        x1 = max(window->x, w->x);
        x2 = min(window->x + window->width, w->x + w->width);
        y1 = max(window->y, w->y);
        y2 = min(window->y + window->height, w->y + w->height);
    }

    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
            if (window->visible[(x - window->x) + (y - window->y) * window->width]) {
                vgui_set_pixel(desktop->vgui, x, y, window->buffer[(x - window->x) + (y - window->y) * window->width]);
            }
        }
    }

    if (w->changed && is_first) {
        window_set_pixels_intersection(desktop, window, w, 0);
    }
}

void window_set_pixels_visible(desktop_t *desktop, window_t *window, int all) {
    if (NO_OPTI) all = 1;
    if (window->changed || all) {
        for (int i = 0; i < window->width; i++) {
            for (int j = 0; j < window->height; j++) {
                if (window->visible[i + j * window->width]) {
                    vgui_set_pixel(desktop->vgui, window->x + i, window->y + j, window->buffer[i + j * window->width]);
                }
            }
        }
        return;
    }

    int total = desktop->nb_windows;
    int *sorted = sort_index_by_priority(desktop->windows, desktop->nb_windows);
    // the first element is the one with the lowest priority

    for (int i = 0; i < total; i++) {
        window_t *w = desktop->windows[sorted[i]];
        if (w == window || !w->changed) continue;

        window_set_pixels_intersection(desktop, window, w, 1);
    }
    free(sorted);
}

int *sort_index_by_priority(window_t **windows, int nb_windows) {
    // the first element is the one with the lowest priority

    int *sorted = calloc(nb_windows, sizeof(int));

    for (int i = 0; i < nb_windows; i++) {
        sorted[i] = i;
    }

    int tmp = 0;

    for (int i = 0; i < nb_windows; i++) {
        for (int j = 0; j < nb_windows; j++) {
            if (windows[j] == NULL) {
                serial_print_ss("BUG", "window is null");
            }
            if (windows[sorted[i]]->priority < windows[sorted[j]]->priority) {
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
    int current_priority = window->priority;
    // the first element is the one with the lowest priority

    for (int i = total - 1; i >= current_priority; i--) {
        if (desktop->windows[sorted[i]] == window) {
            window->priority = total - 1;
        } else {
            desktop->windows[sorted[i]]->priority--;
            desktop->windows[sorted[i]]->changed = 1;
        }
    }

    free(sorted);
}

void window_update_visible(desktop_t *desktop, window_t *window) {
    // can be optimized...

    // update the visible buffer of the window
    // the visible buffer is a buffer of 1 and 0, 1 if the pixel is visible, 0 if not

    // we first set all the pixels to 1
    for (int i = 0; i < window->width * window->height; i++) {
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
        x1 = max(window->x, desktop->windows[sorted[i]]->x);
        x2 = min(window->x + window->width, desktop->windows[sorted[i]]->x + desktop->windows[sorted[i]]->width);
        y1 = max(window->y, desktop->windows[sorted[i]]->y);
        y2 = min(window->y + window->height, desktop->windows[sorted[i]]->y + desktop->windows[sorted[i]]->height);

        for (int x = x1; x < x2; x++) {
            for (int y = y1; y < y2; y++) {
                window->visible[x - window->x + (y - window->y) * window->width] = 0;
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
    // just horizontal or vertical lines
    if (x1 == x2) {
        for (int i = y1; i < y2; i++) {
            window->buffer[x1 + i * window->width] = color;
        }
    } else if (y1 == y2) {
        for (int i = x1; i < x2; i++) {
            window->buffer[i + y1 * window->width] = color;
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
            window->buffer[i + j * window->width] = color;
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
                window->buffer[(i * 8 + x + 8 - k) + (y + j) * window->width] = color;
            }
        }
    }
}
