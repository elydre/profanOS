#ifndef VGUI_ID
#define VGUI_ID 1006

#include <type.h>

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

typedef struct {
    int width;
    int height;
    uint32_t *old_framebuffer;
    uint32_t *framebuffer;
    uint32_t *changed_pixels; // coordinates of changed pixels (y * width + x)
    uint32_t changed_pixels_count;
} vgui_t;

#define vgui_setup(width, height) ((vgui_t (*)(int, int)) get_func_addr(VGUI_ID, 2))(width, height)
#define vgui_exit(vgui) ((void (*)(vgui_t *)) get_func_addr(VGUI_ID, 3))(vgui)
#define vgui_set_pixel(vgui, x, y, color) ((void (*)(vgui_t *, int, int, uint32_t)) get_func_addr(VGUI_ID, 4))(vgui, x, y, color)
#define vgui_get_pixel(vgui, x, y) ((uint32_t (*)(vgui_t *, int, int)) get_func_addr(VGUI_ID, 5))(vgui, x, y)
#define vgui_render(vgui, render_mode) ((void (*)(vgui_t *, int)) get_func_addr(VGUI_ID, 6))(vgui, render_mode)
#define vgui_draw_rect(vgui, x, y, width, height, color) ((void (*)(vgui_t *, int, int, int, int, uint32_t)) get_func_addr(VGUI_ID, 7))(vgui, x, y, width, height, color)
#define vgui_print(vgui, x, y, msg, color) ((void (*)(vgui_t *, int, int, char *, uint32_t)) get_func_addr(VGUI_ID, 8))(vgui, x, y, msg, color)
#define vgui_draw_line(vgui, x1, y1, x2, y2, color) ((void (*)(vgui_t *, int, int, int, int, uint32_t)) get_func_addr(VGUI_ID, 9))(vgui, x1, y1, x2, y2, color)
#define vgui_clear(vgui, color) ((void (*)(vgui_t *, uint32_t)) get_func_addr(VGUI_ID, 10))(vgui, color)
#define vgui_putc(vgui, x, y, c, color, bg_color) ((void (*)(vgui_t *, int, int, char, uint32_t, int)) get_func_addr(VGUI_ID, 11))(vgui, x, y, c, color, bg_color)

#endif
