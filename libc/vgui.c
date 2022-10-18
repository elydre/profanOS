#include <gui/font.h>
#include <gui/vgui.h>
#include <gui/vga.h>
#include <mem.h>

char * last_render;
char * current_render;
int refresh_mode;


void vgui_setup(int refresh_all) {
    vga_320_mode();
    last_render = calloc(320 * 200);
    current_render = calloc(320 * 200);
    refresh_mode = refresh_all + 1;
}

void vgui_exit() {
    vga_text_mode();
    free(last_render);
    free(current_render);
}

void vgui_render() {
    for (int i = 0; i < 320 * 200; i++) {
        if (last_render[i] != current_render[i] || refresh_mode) {
            vga_put_pixel(i % 320, i / 320, current_render[i]);
            last_render[i] = current_render[i];
        }
    }
    refresh_mode = (refresh_mode == 2) ? refresh_mode : 0;
}

void vgui_draw_rect(int x, int y, int width, int height, int color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            current_render[(y + j) * 320 + x + i] = color;
        }
    }
}

void vgui_set_pixel(int x, int y, int color) {
    current_render[y * 320 + x] = color;
}

int vgui_get_pixel(int x, int y) {
    return current_render[y * 320 + x];
}

void vgui_print(int x, int y, char msg[], int big, unsigned color) {
    unsigned char *glyph, *font;
    font = big ? g_8x16_font : g_8x8_font;
    for (int i = 0; msg[i] != '\0'; i++) {
        glyph = font + (msg[i] * (big ? 16 : 8));
        for (int j = 0; j < (big ? 16 : 8); j++) {
            for (int k = 0; k < 8; k++) {
                if (!(glyph[j] & (1 << k))) continue;
                vgui_set_pixel(i * 8 + x + 8 - k , y + j, color);
            }
        }
    }
}
