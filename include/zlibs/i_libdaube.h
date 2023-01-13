#ifndef LIBDAUBE_ID
#define LIBDAUBE_ID 1012

#include <type.h>
#include <stdarg.h>

typedef struct window_t {
    char *name;
    int height;
    int width;
    int x;
    int y;
    uint32_t *buffer;
    int priorite;
} window_t;

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
void input(char out_buffer[], int size, char color);
window_t *window_create(char *name, int height, int width, int y, int x);
void window_draw(window_t *window);
*/

#ifndef LIBDAUBE_C
#define window_create ((window_t *(*)(char *, int, int, int, int)) get_func_addr(LIBDAUBE_ID, 3))
#define window_draw ((void (*)(vgui_t *, window_t *)) get_func_addr(LIBDAUBE_ID, 4))
#endif


#endif
