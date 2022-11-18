#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <mem.h>

#define FONT_TABLE font_get_8x16()
#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#define MAX_COLS (1024 / FONT_WIDTH)
#define MAX_ROWS (768 / FONT_HEIGHT)

int cursor_x = 0;
int cursor_y = 0;


void tef_clear() {
    cursor_x = 0;
    cursor_y = 0;
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 768; j++) {
            vesa_set_pixel(i, j, 0);
        }
    }
}

// a character at a given position
void tef_set_char(int x, int y, char c, uint32_t color, uint32_t bg_color) {
    int i, j;
    for (i = 0; i < FONT_WIDTH; i++) {
        for (j = 0; j < FONT_HEIGHT; j++) {
            if (FONT_TABLE[c * FONT_HEIGHT + j] & (1 << i)) {
                vesa_set_pixel(x + 8 - i, y + j, color);
            } else {
                vesa_set_pixel(x + 8 - i, y + j, bg_color);
            }
        }
    }
}

void tef_print_char(char c, int x, int y, uint32_t color, uint32_t bg_color) {
    if (x != -1) cursor_x = x;
    if (y != -1) cursor_y = y;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y += 1;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == 0x08) {
        // str_backspace
        tef_set_char(cursor_x * FONT_WIDTH, cursor_y * FONT_HEIGHT, ' ', color, bg_color);
    } else {
        tef_set_char(cursor_x * FONT_WIDTH, cursor_y * FONT_HEIGHT, c, color, bg_color);
        cursor_x++;
    }

    if (cursor_x >= MAX_COLS) {
        cursor_x = 0;
        cursor_y++;
    }

    // scroll
    if (cursor_y >= MAX_ROWS) {
        uint32_t *fb = vesa_get_framebuffer();
        mem_move(fb + 1024 * 16, fb, 1024 * 752 * 4);
        for (int i = 0; i < 1024; i++) {
            for (int j = 0; j < 16; j++) {
                vesa_set_pixel(i, 768 - j, 0);
            }
        }
        cursor_y--;
        cursor_x = 0;
        return;
    }

    // update cursor
    // tef_set_char(cursor_x * FONT_WIDTH, cursor_y * FONT_HEIGHT, '_', 0x00FFFF, 0);
}

void tef_print(char *message, int x, int y, uint32_t color, uint32_t bg_color) {
    // if cursor_x or cursor_y is -1, use the current cursor position
    for (int i = 0; message[i] != '\0'; i++) {
        tef_print_char(message[i], x, y, color, bg_color);
        x = -1;
        y = -1;
    }
}


int tef_get_cursor_offset() {
    // we have to multiply by 2 for the text mode compatibility
    return (cursor_y * MAX_COLS + cursor_x) * 2;
}

void tef_set_cursor_offset(int offset) {
    // we have to divide by 2 for the text mode compatibility
    offset /= 2;
    cursor_y = offset / MAX_COLS;
    cursor_x = offset % MAX_COLS;
}
