#include <gui/vesa.h>
#include <gui/font.h>
#include <mem.h>

#define FONT_TABLE g_8x16_font
#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#define COLS (1024 / FONT_HEIGHT)
#define ROWS (768 / FONT_WIDTH)

char char_buffer[COLS * ROWS];
uint32_t color_buffer[COLS * ROWS];

int cursor_x = 0;
int cursor_y = 0;

// a character at a given position
void tef_set_char(int x, int y, char c, uint32_t color) {
    int i, j;
    for (i = 0; i < FONT_WIDTH; i++) {
        for (j = 0; j < FONT_HEIGHT; j++) {
            if (FONT_TABLE[c * FONT_HEIGHT + j] & (1 << i)) {
                vesa_set_pixel(x + i, y + j, color);
            }
        }
    }
}

void tef_redraw() {
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < ROWS; j++) {
            tef_set_char(i * FONT_WIDTH, j * FONT_HEIGHT, char_buffer[i + j * COLS], color_buffer[i + j * COLS]);
        }
    }
}

void tef_print_char(char c, int *x, int *y, uint32_t color) {
    if (c == '\n') {
        *x = 0;
        *y += 1;
    } else if (c == '\r') {
        *x = 0;
    } else if (c == 0x08) {
        // str_backspace
        tef_set_char((*x) * FONT_WIDTH, (*y) * FONT_HEIGHT, ' ', color);
    } else {
        tef_set_char((*x) * FONT_WIDTH, (*y) * FONT_HEIGHT, c, color);
        *x += 1;
    }

    if (*x >= COLS) {
        *x = 0;
        *y += 1;
    }
    if (*y >= ROWS) {
        *y = 0;
    }

    // scroll
    if (*y >= ROWS) {
        for (int i = 0; i < COLS * (ROWS - 1); i++) {
            char_buffer[i] = char_buffer[i + COLS];
            color_buffer[i] = color_buffer[i + COLS];
        }
        for (int i = COLS * (ROWS - 1); i < COLS * ROWS; i++) {
            char_buffer[i] = ' ';
            color_buffer[i] = color;
        }
        *y = ROWS - 1;
        tef_redraw();
    }

    // update cursor
    tef_set_char((*x) * FONT_WIDTH, (*y) * FONT_HEIGHT, '_', 0x00FF00);
}

void tef_print(char *message, int x, int y, uint32_t color) {
    // if cursor_x or cursor_y is -1, use the current cursor position
    if (x == -1) x = cursor_x;
    if (y == -1) y = cursor_y;

    for (int i = 0; message[i] != '\0'; i++) {
        tef_print_char(message[i], &x, &y, color);
    }
}

void tef_clear() {
    for (int i = 0; i < COLS * ROWS; i++) {
        char_buffer[i] = ' ';
        color_buffer[i] = 0xFFFFFF;
    }
    cursor_x = 0;
    cursor_y = 0;
    tef_redraw();
}

int tef_get_cursor_offset() {
    // we have to multiply by 2 for the text mode compatibility
    return (cursor_y * COLS + cursor_x) * 2;
}

void tef_set_cursor_offset(int offset) {
    // we have to divide by 2 for the text mode compatibility
    offset /= 2;
    cursor_y = offset / COLS;
    cursor_x = offset % COLS;
}
