#ifndef VGUI_H
#define VGUI_H

#include <profan/type.h>

typedef struct {
    int width;
    int height;
    uint32_t *old_framebuffer;
    uint32_t *framebuffer;
    uint32_t *changed_pixels; // coordinates of changed pixels (y * width + x)
    int changed_pixels_count;
} vgui_t;

vgui_t vgui_setup(int width, int height);
void vgui_exit(vgui_t *vgui);
void vgui_set_pixel(vgui_t *vgui, int x, int y, uint32_t color);
uint32_t vgui_get_pixel(vgui_t *vgui, int x, int y);
void vgui_render(vgui_t *vgui, int render_mode);
void vgui_draw_rect(vgui_t *vgui, int x, int y, int width, int height, uint32_t color);
void vgui_print(vgui_t *vgui, int x, int y, char *msg, uint32_t color);
void vgui_draw_line(vgui_t *vgui, int x1, int y1, int x2, int y2, uint32_t color);
void vgui_clear(vgui_t *vgui, uint32_t color);
void vgui_putc(vgui_t *vgui, int x, int y, char c, uint32_t color, int bg_color);

#endif
