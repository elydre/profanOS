#include <syscall.h>
#include <stdlib.h>
#include <string.h>

#include <i_libdaube.h>

#define WADDS_NOBG 0xFF000000

int main() {
    return 0;
}

button_t *wadds_create_exitbt(window_t *window, void (*exit_callback)(clickevent_t *)) {
    int x = window->width - 3;

    // draw bg rectangle
    for (int i = 3; i < 16; i++) {
        for (int j = 3; j < 16; j++) {
            window_set_pixel_out(window, x - i, j, 0x880000);
        }
    }

    // draw cross
    for (int i = 3; i < 16; i++) {
        for (int j = -1; j < 2; j++) {
            window_set_pixel_out(window, x - i, (i + j != 2) ? i + j : 3, 0xa7a0b9);
            window_set_pixel_out(window, x - (19 - i), (i + j != 2) ? i + j : 3, 0xa7a0b9);
        }
    }

    // draw outlines
    for (int i = 3; i < 16; i++) {
        window_set_pixel_out(window, x - i, 3, 0x880000);
        window_set_pixel_out(window, x - i - 1, 16, 0x880000);
        window_set_pixel_out(window, x - 3, i + 1, 0x880000);
        window_set_pixel_out(window, x - 16, i, 0x880000);
    }

    return create_button(window, x - 18, 3, 16, 16, exit_callback);
}

void wadds_line(window_t *window, int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        window_set_pixel(window, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void wadds_rect(window_t *window, int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            window_set_pixel(window, x + i, y + j, color);
        }
    }
}

void wadds_putc(window_t *window, int x, int y, char c, uint32_t color, uint32_t bg_color) {
    uint8_t *glyph = c_font_get(0) + c * 16;
    for (int j = 0; j < 16; j++) {
        for (int k = 0; k < 8; k++) {
            if (!(glyph[j] & (1 << k)) && bg_color == WADDS_NOBG) continue;
            window_set_pixel(window, x + 8 - k, y + j, (glyph[j] & (1 << k)) ? color : bg_color);
        }
    }
}

void wadds_puts(window_t *window, int x, int y, char *str, uint32_t color, uint32_t bg_color) {
    for (uint32_t i = 0; i < strlen(str); i++) {
        wadds_putc(window, x + i * 8, y, str[i], color, bg_color);
    }
}

void wadds_fill(window_t *window, uint32_t color) {
    for (int i = 0; i < window->width; i++) {
        for (int j = 0; j < window->height; j++) {
            window_set_pixel(window, i, j, color);
        }
    }
}
