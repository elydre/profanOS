#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <i_libdaube.h>

#define WADDS_NOBG 0xFF000000

int main() {
    return 0;
}

void wadds_line(window_t *window, int x1, int y1, int x2, int y2, uint32_t color, uint8_t inst) {
    int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        if (!inst) window_set_pixel(window, x1, y1, color);
        else window_display_pixel(window, x1, y1, color);
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

void wadds_rect(window_t *window, int x, int y, int width, int height, uint32_t color, uint8_t inst) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            if (!inst) window_set_pixel(window, x + i, y + j, color);
            else window_display_pixel(window, x + i, y + j, color);
        }
    }
}

void wadds_putc(window_t *window, char c, int x, int y, uint32_t color, uint32_t bg_color, uint8_t inst) {
    uint8_t *glyph = c_font_get(0) + c * 16;
    for (int j = 0; j < 16; j++) {
        for (int k = 0; k < 8; k++) {
            if (!(glyph[j] & (1 << k)) && bg_color == WADDS_NOBG) continue;
            if (!inst) window_set_pixel(window, x + 8 - k, y + j, (glyph[j] & (1 << k)) ? color : bg_color);
            else window_display_pixel(window, x + 8 - k, y + j, (glyph[j] & (1 << k)) ? color : bg_color);
        }
    }
}

void wadds_puts(window_t *window, char *str, int x, int y, uint32_t color, uint32_t bg_color, uint8_t inst) {
    for (uint32_t i = 0; i < strlen(str); i++) {
        wadds_putc(window, str[i], x + i * 8, y, color, bg_color, inst);
    }
}

void wadds_fill(window_t *window, uint32_t color, uint8_t inst) {
    for (int i = 0; i < window->width; i++) {
        for (int j = 0; j < window->height; j++) {
            if (!inst) window_set_pixel(window, i, j, color);
            else window_display_pixel(window, i, j, color);
        }
    }
}

int wadds_get_kb(window_t *window) {
    // get the desktop
    desktop_t *desktop = (window == NULL) ?
        desktop_get_main() :
        (desktop_t *) window->parent_desktop;

    // if the default window is focused
    if (!desktop->key_state[0] &&
        ((desktop->focus_window_usid == window->usid) ||
        (window == NULL && desktop->focus_window_usid == 0)))
        return c_kb_get_scfh();

    return 0;
}

/*****************************
 *   draw bmp error codes   *
 * 1: file doesn't exist    *
 * 2: file isn't a bmp      *
 * 3: window is too small   *
 * 4: width or height is 0  *
 * 5: file is not 24/32 bit *
*****************************/

int wadds_draw_bmp(window_t *window, char *path, int x, int y) {
    // check if file exists
    if (!(c_fs_does_path_exists(path) && c_fs_get_sector_type(c_fs_path_to_id(path)) == 2))
        return 1;

    // open file
    uint8_t *file_content = c_fs_declare_read_array(path);
    c_fs_read_file(path, file_content);

    // check if file is a bmp
    if (file_content[0] != 'B' || file_content[1] != 'M') {
        free(file_content);
        return 2;
    }

    // get image data
    int width = *(int *)(file_content + 18);
    int height = *(int *)(file_content + 22);
    int offset = *(int *)(file_content + 10);
    int size = *(int *)(file_content + 34);
    uint8_t *data = file_content + offset;

    if (window->width < x + width || window->height < y + height) {
        free(file_content);
        return 3;
    }

    if (width <= 0 || height <= 0) {
        free(file_content);
        return 4;
    }

    int factor = size / (width * height);

    // draw image
    if (factor != 3 && factor != 4) {
        free(file_content);
        return 5;
    }

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            uint32_t color = data[(j * width + i) * factor] |
                            (data[(j * width + i) * factor + 1] << 8) |
                            (data[(j * width + i) * factor + 2] << 16);

            if (factor == 4 && data[(j * width + i) * factor + 3] == 0) continue;
            window_set_pixel(window, x + i, (y + height - 1) - j, color);
        }
    }

    free(file_content);
    return 0;
}
