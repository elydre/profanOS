#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <i_time.h>

#define LIBDAUBE_C
#include <i_libdaube.h>

#define NO_OPTI      0 // disable optimizations

#define DEBUG_LEVEL  0
#define MOVE_SPEED   5

#define COLOR_MASTER 0x6b6e8c
#define COLOR_TITLES 0xa7a0b9

#define COLOR_ACCENT 0xFF0033

#define COLOR_GRADN1 0x120434
#define COLOR_GRADN2 0x00000c

#define COLOR_FOCUS1 0x240865
#define COLOR_FOCUS2 0x0c0f1d

#define WINDOW_MAGIC 0xdeadbeef

#define check_window_fail(w) (w->magic != WINDOW_MAGIC)

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

desktop_t *main_desktop;

#define ID_DESKTOP_REFRESH  1
#define ID_WINDOW_DELETE    2

#define KEY_F5      0x3F
#define KEY_TAB     0x0F
#define KEY_LEFT    0x4B
#define KEY_RIGHT   0x4D
#define KEY_UP      0x48
#define KEY_DOWN    0x50
#define KEY_ALT     0x38

#define KEY_RELEASE 0x80

void init_func();

int main() {
    init_func();
    return 0;
}

void init_func() {
    main_desktop = NULL;
    printf("Init of the libdaube library\n");
}

window_t *window_create(desktop_t* desktop, char *name, int x, int y, int width, int height, uint8_t is_lite, uint8_t cant_move, uint8_t always_on_top);

void window_set_pixels_visible(desktop_t *desktop, window_t *window, int all);
void window_update_visible(desktop_t *desktop, window_t *window);
int *sort_index_by_priority(window_t **windows, int nb_windows);
void set_window_priority(desktop_t *desktop, window_t *window);
void window_draw_box(desktop_t *desktop, window_t *window);
int focus_window(desktop_t *desktop, window_t *window);
void desktop_patch_always_on_top(desktop_t *desktop);
void func_desktop_refresh(desktop_t *desktop);
void serial_print_ss(char *str, char *name);

void draw_rect_gradian(window_t *window, int x, int y, int width, int height, int color1, int color2);
void draw_straight_line(window_t *window, int x1, int y1, int x2, int y2, int color);
void draw_rect(window_t *window, int x, int y, int width, int height, int color);
void draw_print_wut(window_t *window, int x, int y, char *msg, int color);
void window_draw_temp_box(window_t *window, int x, int y, uint8_t mode);
int usid_to_id(desktop_t *desktop, int usid);


desktop_t *desktop_init(int max_windows, int screen_width, int screen_height) {
    desktop_t *desktop = calloc(1, sizeof(desktop_t));

    desktop->windows = calloc(max_windows, sizeof(window_t *));
    desktop->max_windows = max_windows;
    desktop->nb_windows = 0;

    desktop->screen_buffer = malloc(screen_width * screen_height * sizeof(uint32_t));
    for (int i = 0; i < screen_width * screen_height; i++) {
        desktop->screen_buffer[i] = 0xFF0000;
    }

    desktop->screen_width = screen_width;
    desktop->screen_height = screen_height;

    desktop->current_usid = 0;
    desktop->focus_window_usid = 0;

    desktop->func_run_stack = calloc(100, sizeof(libdaude_func_t));
    desktop->func_run_stack_size = 0;

    desktop->refresh_data = calloc(1, sizeof(refresh_ui_t));

    if (main_desktop == NULL) {
        main_desktop = desktop;
    }

    return desktop;
}

window_t *window_create(desktop_t* desktop, char *name, int x, int y, int width, int height, uint8_t is_lite, uint8_t cant_move, uint8_t always_on_top) {
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

    window->usid = desktop->current_usid++;
    window->priority = window->usid;

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
    window->buffer = calloc(window->height * window->width, sizeof(uint32_t));
    window->visible = calloc(window->height * window->width, sizeof(uint8_t));

    if (!is_lite) {
        window->topbar_buffer = calloc(window->width * 19, sizeof(uint32_t));
    } else {
        window->topbar_buffer = NULL;
    }

    window->changed = 1;

    window->magic = WINDOW_MAGIC;

    window->always_on_top = always_on_top;
    window->cant_move = cant_move;
    window->is_lite = is_lite;

    // draw the border of the window in the buffer
    window_draw_box(desktop, window);

    // add the window to the desktop
    for (int i = 0; i < desktop->max_windows; i++) {
        if (desktop->windows[i] == NULL) {
            desktop->windows[i] = window;
            break;
        }
    }

    desktop->nb_windows++;

    desktop_patch_always_on_top(desktop);

    return window;
}

void desktop_refresh(desktop_t *desktop) {
    // check if the function is already in the stack
    for (int i = 0; i < desktop->func_run_stack_size; i++) {
        if (desktop->func_run_stack[i].func_id == ID_DESKTOP_REFRESH && desktop->func_run_stack[i].arg1 == (uint32_t) desktop) {
            return;
        }
    }

    int index = desktop->func_run_stack_size;

    desktop->func_run_stack[index].func_id = ID_DESKTOP_REFRESH;
    desktop->func_run_stack[index].arg1 = (uint32_t) desktop;
    desktop->func_run_stack_size++;
}

void window_move(window_t *window, int x, int y) {
    window->old_x = window->x;
    window->old_y = window->y;

    window->x = x;
    window->y = y;

    if (window->is_lite) {
        window->in_y = y + 1;
        window->in_x = x + 1;
    } else {
        window->in_y = y + 21;
        window->in_x = x + 6;
    }

    window->changed = 1;
}

void window_resize(window_t *window, int width, int height) {
    // TODO
}

void window_set_pixel_func(window_t *window, int x, int y, uint32_t color, uint8_t in_box) {
    if (check_window_fail(window)) {
        serial_print_ss("error:", "window_set_pixel: invalid window");
        return;
    }

    // the coordinates are those of the interior of the window
    int out_x = x;
    int out_y = y;


    if (in_box) {
        if (x >= window->in_width || y >= window->in_height) return;
        out_x += window->in_x - window->x;
        out_y += window->in_y - window->y;
    }

    if (out_x < 0 || out_x >= window->width || out_y < 0 || out_y >= window->height) return;

    if (!in_box && window->topbar_buffer != NULL && out_y != 0 && out_y < 20) {
        window->topbar_buffer[out_y * window->width + out_x] = 0xFF000000 ^ color;
    }

    window->buffer[out_y * window->width + out_x] = color;
}

void window_display_pixel_func(window_t *window, int x, int y, uint32_t color, uint8_t in_box) {
    if (check_window_fail(window)) {
        serial_print_ss("error:", "window_set_pixel: invalid window");
        return;
    }

    // the coordinates are those of the interior of the window
    int out_x = x;
    int out_y = y;

    if (in_box) {
        if (x >= window->in_width || y >= window->in_height) return;
        out_x += window->in_x - window->x;
        out_y += window->in_y - window->y;
    }

    if (out_x < 0 || out_x >= window->width || out_y < 0 || out_y >= window->height) return;

    if (!in_box && window->topbar_buffer != NULL && out_y != 0 && out_y < 20) {
        window->topbar_buffer[out_y * window->width + out_x] = 0xFF000000 ^ color;
    }

    window->buffer[out_y * window->width + out_x] = color;

    desktop_t *desktop = window->parent_desktop;

    if (!window->visible[out_y * window->width + out_x]) {
        return;
    }

    uint32_t screen_buffer_index = (window->x + out_x) + (window->y + out_y) * desktop->screen_width;

    if (desktop->screen_buffer[screen_buffer_index] == window->buffer[out_y * window->width + out_x]) {
        return;
    }

    c_vesa_set_pixel(window->x + out_x, window->y + out_y, window->buffer[out_y * window->width + out_x]);
    desktop->screen_buffer[screen_buffer_index] = window->buffer[out_y * window->width + out_x];
}

void window_refresh(window_t *window) {
    if (check_window_fail(window)) {
        serial_print_ss("error:", "window_refresh: invalid window");
        return;
    }

    desktop_t *desktop = window->parent_desktop;

    while (desktop->is_locked) {
        if (DEBUG_LEVEL > 2) serial_print_ss("desktop is locked, can't refresh", window->name);
        ms_sleep(10);
    }

    while (window->changed) {
        if (DEBUG_LEVEL > 2) serial_print_ss("window is changed, can't refresh", window->name);
        ms_sleep(10);
    }

    window_set_pixels_visible(desktop, window, 1);
}

void refresh_ui(desktop_t *desktop) {
    refresh_ui_t *rdata = desktop->refresh_data;

    window_t *window;

    // get the keyboard input:
    // F5 + tab -> switch between windows (stop when you release F5)
    // F5 + arrow -> move window          (stop when you release F5)

    // key_state[0] = F5
    // key_state[1] = tab
    // key_state[2] = left
    // key_state[3] = right
    // key_state[4] = up
    // key_state[5] = down

    if ((c_kb_get_scancode() == KEY_F5 || c_kb_get_scancode() == KEY_ALT) && !desktop->key_state[0]) {
        if (rdata->last_window_index >= desktop->nb_windows) {
            rdata->last_window_index = 0;
        }

        desktop->key_state[0] = 1;

        window = desktop->windows[rdata->last_window_index];

        rdata->last_x = window->x;
        rdata->last_y = window->y;

        window_draw_temp_box(window, rdata->last_x, rdata->last_y, 0);

        return;
    }

    if (!desktop->key_state[0]) {
        return;
    }

    switch (c_kb_get_scfh()) {  // get from history
        case KEY_TAB:
            desktop->key_state[1] = 1;
            break;
        case KEY_LEFT:
            desktop->key_state[2] = 1;
            break;
        case KEY_RIGHT:
            desktop->key_state[3] = 1;
            break;
        case KEY_UP:
            desktop->key_state[4] = 1;
            break;
        case KEY_DOWN:
            desktop->key_state[5] = 1;
            break;
        case KEY_F5 + KEY_RELEASE:
            desktop->key_state[0] = 0;
            break;
        case KEY_ALT + KEY_RELEASE:
            desktop->key_state[0] = 0;
            break;
        case KEY_TAB + KEY_RELEASE:
            desktop->key_state[1] = 0;
            break;
        case KEY_LEFT + KEY_RELEASE:
            desktop->key_state[2] = 0;
            break;
        case KEY_RIGHT + KEY_RELEASE:
            desktop->key_state[3] = 0;
            break;
        case KEY_UP + KEY_RELEASE:
            desktop->key_state[4] = 0;
            break;
        case KEY_DOWN + KEY_RELEASE:
            desktop->key_state[5] = 0;
            break;
    }

    if (rdata->last_window_index >= desktop->nb_windows) {
        rdata->last_window_index = 0;
    }

    // F5 released
    if (!desktop->key_state[0]) {
        window = desktop->windows[rdata->last_window_index];
        window_draw_temp_box(window, rdata->last_x, rdata->last_y, 1);

        if (window->usid != desktop->focus_window_usid) {
            focus_window(desktop, window);
            desktop_refresh(desktop);
        } if (rdata->window_moving) {
            window_move(window, rdata->last_x, rdata->last_y);
            rdata->window_moving = 0;
            desktop_refresh(desktop);
        }
        return;
    }

    // tab
    if (desktop->key_state[1]) {
        desktop->key_state[1] = 0;
        int window_index = rdata->last_window_index + 1;

        if (window_index >= desktop->nb_windows) {
            window_index = 0;
        }

        window = desktop->windows[rdata->last_window_index];
        window_draw_temp_box(window, rdata->last_x, rdata->last_y, 1);    // erase old temp box

        rdata->last_window_index = window_index;
        window = desktop->windows[rdata->last_window_index];

        rdata->last_x = window->x;
        rdata->last_y = window->y;

        window_draw_temp_box(window, rdata->last_x, rdata->last_y, 0);    // draw new temp box
    }

    // direction
    if (desktop->key_state[2] || desktop->key_state[3] ||
        desktop->key_state[4] || desktop->key_state[5]
    ) {
        window = desktop->windows[rdata->last_window_index];

        if (window->cant_move) {
            return;
        }

        rdata->window_moving = 1;

        window_draw_temp_box(window, rdata->last_x, rdata->last_y, 1);

        if (desktop->key_state[2]) rdata->last_x -= MOVE_SPEED;
        if (desktop->key_state[3]) rdata->last_x += MOVE_SPEED;
        if (desktop->key_state[4]) rdata->last_y -= MOVE_SPEED;
        if (desktop->key_state[5]) rdata->last_y += MOVE_SPEED;

        if (rdata->last_x <= 0) rdata->last_x = desktop->screen_width - 1;
        if (rdata->last_x >= desktop->screen_width) rdata->last_x = 0;
        if (rdata->last_y <= 0) rdata->last_y = desktop->screen_height - 1;
        if (rdata->last_y >= desktop->screen_height) rdata->last_y = 0;

        // draw temp outline
        window_draw_temp_box(window, rdata->last_x, rdata->last_y, 0);
    }
}

desktop_t *desktop_get_main() {
    return main_desktop;
}

void window_delete(window_t *window) {
    desktop_t *desktop = window->parent_desktop;
    int index = desktop->func_run_stack_size;

    desktop->func_run_stack[index].func_id = ID_WINDOW_DELETE;
    desktop->func_run_stack[index].arg1 = (uint32_t) window;
    desktop->func_run_stack_size++;
}

void window_wait_delete(desktop_t *desktop, window_t *window) {
    int found = 1;
    while (found) {
        found = 0;
        for (int i = 0; i < desktop->nb_windows; i++) {
            if (desktop->windows[i] == window) {
                found = 1;
                break;
            }
        }
        ms_sleep(10);
    }
}

/********************************
 *                             *
 *      FUNC CALL SYSTEM       *
 *                             *
********************************/

void func_desktop_refresh(desktop_t *desktop);
void func_window_delete(window_t *window);

void desktop_run_stack(desktop_t *desktop) {
    char tmp[4];
    libdaude_func_t func_run;
    while (desktop->func_run_stack_size > 0) {
        func_run = desktop->func_run_stack[desktop->func_run_stack_size - 1];
        desktop->func_run_stack_size--;
        itoa(func_run.func_id, tmp, 10);
        if (DEBUG_LEVEL > 2) serial_print_ss("runing func:", tmp);
        switch (func_run.func_id) {
            case ID_DESKTOP_REFRESH:
                func_desktop_refresh((desktop_t *) func_run.arg1);
                break;
            case ID_WINDOW_DELETE:
                func_window_delete((window_t *) func_run.arg1);
                break;
            default:
                serial_print_ss("BUG", "func not found");
                break;
        }
    }
}

/******************************
 *                           *
 *    INTERNAL FUNCTIONS     *
 *                           *
******************************/

void func_desktop_refresh(desktop_t *desktop) {
    if (desktop->is_locked) return;
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

    free(sorted);

    desktop->is_locked = 0;
}

void func_window_delete(window_t *window) {
    // we need to refresh the desktop
    desktop_t *desktop = window->parent_desktop;
    window->priority = -1;
    window->changed = 1;

    func_desktop_refresh(window->parent_desktop);

    free(window->name);
    free(window->buffer);
    free(window->visible);

    if (window->topbar_buffer != NULL) {
        free(window->topbar_buffer);
    }

    // remove the window
    for (int i = 0; i < desktop->nb_windows; i++) {
        if (desktop->windows[i] == window) {
            if (i != desktop->nb_windows - 1) {
                desktop->windows[i] = desktop->windows[desktop->nb_windows - 1];
                break;
            }
            desktop->windows[i] = NULL;
        }
    }

    desktop->nb_windows--;

    free(window);
}

void window_set_pixels_intersection(desktop_t *desktop, window_t *window, window_t *w, uint8_t is_first) {
    int x1, x2, y1, y2, win_buffer_index;

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
            win_buffer_index = (x - window->x) + (y - window->y) * window->width;

            if (x < 0 || x >= desktop->screen_width || y < 0 || y >= desktop->screen_height) {
                continue;
            }

            if (! window->visible[win_buffer_index]) {
                continue;
            }

            if (desktop->screen_buffer[x + y * desktop->screen_width] == window->buffer[win_buffer_index]) {
                continue;
            }

            c_vesa_set_pixel(x, y, window->buffer[win_buffer_index]);
            desktop->screen_buffer[x + y * desktop->screen_width] = window->buffer[win_buffer_index];
        }
    }

    if (w->changed && is_first) {
        window_set_pixels_intersection(desktop, window, w, 0);
    }
}

void window_set_pixels_visible(desktop_t *desktop, window_t *window, int all) {
    if (NO_OPTI) all = 1;

    if (window->changed || all) {
        int screen_buffer_index;
        for (int i = 0; i < window->width; i++) {
            for (int j = 0; j < window->height; j++) {
                if (window->x + i < 0 || window->x + i >= desktop->screen_width || window->y + j < 0 || window->y + j >= desktop->screen_height) {
                    continue;
                }
                if (!window->visible[i + j * window->width]) {
                    continue;
                }
                screen_buffer_index = (window->x + i) + (window->y + j) * desktop->screen_width;

                if (desktop->screen_buffer[screen_buffer_index] == window->buffer[i + j * window->width]) {
                    continue;
                }

                c_vesa_set_pixel(window->x + i, window->y + j, window->buffer[i + j * window->width]);
                desktop->screen_buffer[screen_buffer_index] = window->buffer[i + j * window->width];
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
    int is_always_on_top = window->always_on_top;

    // the first element is the one with the lowest priority

    int new_priority = total - 1;
    for (int i = total - 1; i >= current_priority; i--) {
        if (!is_always_on_top && desktop->windows[sorted[i]]->always_on_top) {
            new_priority = i - 1;
            continue;
        }
        if (desktop->windows[sorted[i]] == window) {
            window->priority = new_priority;
        } else {
            desktop->windows[sorted[i]]->priority--;
            desktop->windows[sorted[i]]->changed = 1;
        }
    }
    free(sorted);
}

int focus_window(desktop_t *desktop, window_t *window) {
    int old_focus_id = usid_to_id(desktop, desktop->focus_window_usid);

    if (desktop->focus_window_usid == window->usid) {
        return 0;
    }

    desktop->focus_window_usid = window->usid;

    if (old_focus_id != -1) {
        window_draw_box(desktop, desktop->windows[old_focus_id]);
        desktop->windows[old_focus_id]->changed = 1;
    }

    window_draw_box(desktop, window);
    window->changed = 1;

    return 1;
}

void desktop_patch_always_on_top(desktop_t *desktop) {
    // patch the windows priority on creation

    int total = desktop->nb_windows;
    int *sorted = sort_index_by_priority(desktop->windows, total);
    // the first element is the one with the lowest priority

    for (int i = 0; i < total; i++) {
        if (! desktop->windows[sorted[i]]->always_on_top) continue;
        if (desktop->windows[sorted[i]]->priority != total - 1) {
            set_window_priority(desktop, desktop->windows[sorted[i]]);
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

int usid_to_id(desktop_t *desktop, int usid) {
    for (int i = 0; i < desktop->nb_windows; i++) {
        if (desktop->windows[i]->usid == usid) {
            return i;
        }
    }
    return -1;
}

void serial_print_ss(char *str, char *name) {
    c_serial_print(SERIAL_PORT_A, str);
    c_serial_print(SERIAL_PORT_A, " ");
    c_serial_print(SERIAL_PORT_A, name);
    c_serial_print(SERIAL_PORT_A, "\n");
}

/******************************
 *                           *
 *     DRAWING FUNCTIONS     *
 *                           *
******************************/

void window_draw_box(desktop_t *desktop, window_t *window) {
    uint8_t is_focus = (window->usid == desktop->focus_window_usid);

    uint32_t grn1 = (is_focus) ? COLOR_FOCUS1 : COLOR_GRADN1;
    uint32_t grn2 = (is_focus) ? COLOR_FOCUS2 : COLOR_GRADN2;

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
        draw_rect_gradian(window, 5, 1, window->width - 9, 19, grn1, grn2);
        draw_rect_gradian(window, 5, window->height - 5, window->width - 9, 4, grn1, grn2);

        // and the rectangles
        draw_rect(window, 1, 1, 4, window->height - 2, grn1);
        draw_rect(window, window->width - 5, 1, 4, window->height - 2, grn2);

        // we add the name of the window
        draw_print_wut(window, 6, 4, window->name, COLOR_TITLES);
    }

    // restore the topbar
    if (window->topbar_buffer == NULL) return;

    for (int i = 0; i < window->width * 19; i++) {
        if (!window->topbar_buffer[i] && 0xFF000000) continue;
        window->buffer[i + window->width] = window->topbar_buffer[i];
    }
}

void window_draw_temp_box(window_t *window, int x, int y, uint8_t mode) {
    // mode 0: COLOR_MASTER
    // mode 1: ERAZE

    int x_max = c_vesa_get_width();
    int y_max = c_vesa_get_height();

    uint32_t *screen_buffer = ((desktop_t *) window->parent_desktop)->screen_buffer;
    int screen_width = ((desktop_t *) window->parent_desktop)->screen_width;

    if (mode == 0) {
        for (int i = x; i < window->width + x - 1; i++) {
            if (i < 0 || i >= x_max) continue;
            if (y >= 0 && y < y_max) c_vesa_set_pixel(i, y, COLOR_ACCENT);
            if (y + window->height - 1 < 0 || y + window->height - 1 >= y_max) continue;
            c_vesa_set_pixel(i, y + window->height - 1, COLOR_ACCENT);
        }
        for (int i = y; i < window->height + y; i++) {
            if (i < 0 || i >= y_max) continue;
            if (x >= 0 && x < x_max) c_vesa_set_pixel(x, i, COLOR_ACCENT);
            if (x + window->width - 1 < 0 || x + window->width - 1 >= x_max) continue;
            c_vesa_set_pixel(x + window->width - 1, i, COLOR_ACCENT);
        }
        return;
    } else if (mode == 1) {
        for (int i = x; i < window->width + x - 1; i++) {
            if (i < 0 || i >= x_max) continue;
            if (y >= 0 && y < y_max) c_vesa_set_pixel(i, y, screen_buffer[i + y * screen_width]);
            if (y + window->height - 1 < 0 || y + window->height - 1 >= y_max) continue;
            c_vesa_set_pixel(i, y + window->height - 1, screen_buffer[i + (y + window->height - 1) * screen_width]);
        }
        for (int i = y; i < window->height + y; i++) {
            if (i < 0 || i >= y_max) continue;
            if (x >= 0 && x < x_max) c_vesa_set_pixel(x, i, screen_buffer[x + i * screen_width]);
            if (x + window->width - 1 < 0 || x + window->width - 1 >= x_max) continue;
            c_vesa_set_pixel(x + window->width - 1, i, screen_buffer[(x + window->width - 1) + i * screen_width]);
        }
    }
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
