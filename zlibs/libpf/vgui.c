/*****************************************************************************\
|   === vgui.c : 2024 ===                                                     |
|                                                                             |
|    Double-buffered basic graphics library for profanOS           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan/vgui.h>

#include <stdlib.h>

vgui_t vgui_setup(int width, int height) {
    vgui_t vgui;
    int buffer_size = width * height;
    vgui.width = width;
    vgui.height = height;

    vgui.old_framebuffer = calloc(buffer_size, sizeof(uint32_t));
    vgui.framebuffer = calloc(buffer_size, sizeof(uint32_t));
    vgui.changed_pixels = calloc(buffer_size, sizeof(uint32_t));

    vgui_render(&vgui, 1);
    return vgui;
}

void vgui_exit(vgui_t *vgui) {
    free(vgui->old_framebuffer);
    free(vgui->framebuffer);
    free(vgui->changed_pixels);
}

void vgui_set_pixel(vgui_t *vgui, int x, int y, uint32_t color) {
    if (x < 0 || x >= vgui->width || y < 0 || y >= vgui->height) return;
    uint32_t poss = y * vgui->width + x;
    int already_changed = 0;
    if (vgui->framebuffer[poss] != vgui->old_framebuffer[poss]) {
        already_changed = 1;
    }
    vgui->framebuffer[poss] = color;

    if (vgui->framebuffer[poss] != vgui->old_framebuffer[poss] &&
        !already_changed &&
        vgui->changed_pixels_count < vgui->width * vgui->height
    ) vgui->changed_pixels[vgui->changed_pixels_count++] = poss;
}

uint32_t vgui_get_pixel(vgui_t *vgui, int x, int y) {
    if (x < 0 || x >= vgui->width || y < 0 || y >= vgui->height) return 0;
    return vgui->framebuffer[y * vgui->width + x];
}

void vgui_render(vgui_t *vgui, int render_mode) {
    uint32_t *fb = syscall_vesa_fb();
    uint32_t pitch = syscall_vesa_pitch();
    if (render_mode) {
        for (int i = 0; i < vgui->width * vgui->height; i++) {
            fb[i % vgui->width + i / vgui->width * pitch] = vgui->framebuffer[i];
            vgui->old_framebuffer[i] = vgui->framebuffer[i];
        }
    } else {
        int poss;
        for (int i = 0; i < vgui->changed_pixels_count; i++) {
            poss = vgui->changed_pixels[i];
            if (vgui->framebuffer[poss] != vgui->old_framebuffer[poss]) {
                fb[poss % vgui->width + poss / vgui->width * pitch] = vgui->framebuffer[poss];
                vgui->old_framebuffer[poss] = vgui->framebuffer[poss];
            }
        }
    }
    vgui->changed_pixels_count = 0;
}

void vgui_draw_rect(vgui_t *vgui, int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            vgui_set_pixel(vgui, x + i, y + j, color);
        }
    }
}

void vgui_print(vgui_t *vgui, int x, int y, char *msg, uint32_t color) {
    unsigned char *glyph;
    for (int i = 0; msg[i] != '\0'; i++) {
        glyph = syscall_font_get() + msg[i] * 16;
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 8; k++) {
                if (!(glyph[j] & (1 << k))) continue;
                vgui_set_pixel(vgui, i * 8 + x + 8 - k , y + j, color);
            }
        }
    }
}


void vgui_draw_line(vgui_t *vgui, int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int xinc = (x2 > x1) ? 1 : -1;
    int yinc = (y2 > y1) ? 1 : -1;

    int cumul;
    int i;

    if (dx > dy) {
        cumul = dx / 2;
        for (i = 0; i < dx; i++) {
            x1 += xinc;
            cumul += dy;
            if (cumul >= dx) {
                cumul -= dx;
                y1 += yinc;
            }
            vgui_set_pixel(vgui, x1, y1, color);
        }
    } else {
        cumul = dy / 2;
        for (i = 0; i < dy; i++) {
            y1 += yinc;
            cumul += dx;
            if (cumul >= dy) {
                cumul -= dy;
                x1 += xinc;
            }
            vgui_set_pixel(vgui, x1, y1, color);
        }
    }
}

void vgui_clear(vgui_t *vgui, uint32_t color) {
    for (int i = 0; i < vgui->width; i++) {
        for (int j = 0; j < vgui->height; j++) {
            vgui_set_pixel(vgui, i, j, color);
        }
    }
}

void vgui_putc(vgui_t *vgui, int x, int y, char c, uint32_t color, int bg_color) {
    unsigned char *glyph;
    glyph = syscall_font_get() + c * 16;
    for (int j = 0; j < 16; j++) {
        for (int k = 0; k < 8; k++) {
            if (!(glyph[j] & (1 << k))) {
                if (bg_color == -1) continue;
                vgui_set_pixel(vgui, x + 8 - k, y + j, bg_color);
                continue;
            }
            vgui_set_pixel(vgui, x + 8 - k, y + j, color);
        }
    }
}
