#ifndef LIBDAUBE_ID
#define LIBDAUBE_ID 1015

#include <type.h>
#include <stdarg.h>
#include <i_vgui.h> 

typedef struct window_t {
    char *name;

    int out_x;
    int out_y;
    int out_width;
    int out_height;

    int in_x;
    int in_y;
    int in_width;
    int in_height;

    uint32_t *buffer;
    uint8_t *visible;
    int priorite;
    int is_lite;    // no border
} window_t;

typedef struct desktop_t {
    window_t **windows;
    int nb_windows;
    vgui_t *vgui;
} desktop_t;

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
void input(char out_buffer[], int size, char color);
window_t *window_create(char *name, int height, int width, int y, int x);
void window_draw_box(window_t *window);
void desktop_draw(vgui_t *vgui, desktop_t *desktop);
*/

#ifndef LIBDAUBE_C
#define window_create ((window_t *(*)(desktop_t *, char *, int, int, int, int, int, int)) get_func_addr(LIBDAUBE_ID, 3))
#define window_draw_box ((void (*)(vgui_t *, window_t *)) get_func_addr(LIBDAUBE_ID, 4))
#define desktop_refresh ((void (*)(desktop_t *)) get_func_addr(LIBDAUBE_ID, 5))
#define window_move ((void (*)(window_t *, int, int)) get_func_addr(LIBDAUBE_ID, 6))
#define window_resize ((void (*)(window_t *, int, int)) get_func_addr(LIBDAUBE_ID, 7))
#define window_fill ((void (*)(window_t *, uint32_t)) get_func_addr(LIBDAUBE_ID, 8))
#define window_refresh ((void (*)(desktop_t *, window_t *)) get_func_addr(LIBDAUBE_ID, 9))

#endif

#endif
