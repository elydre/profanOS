#include <gui/gentxt.h>
#include <gui/vesa.h>
#include <gui/font.h>
#include <mem.h>

#define FONT_TABLE g_8x16_font
#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#define MAX_COLS (1024 / FONT_WIDTH)
#define MAX_ROWS (768 / FONT_HEIGHT)

char char_buffer[MAX_COLS * MAX_ROWS];
uint32_t color_buffer[MAX_COLS * MAX_ROWS];

int cursor_x = 0;
int cursor_y = 0;

// a character at a given position
void tef_set_char(int x, int y, char c, uint32_t color) {
    int i, j;
    for (i = 0; i < FONT_WIDTH; i++) {
        for (j = 0; j < FONT_HEIGHT; j++) {
            if (FONT_TABLE[c * FONT_HEIGHT + j] & (1 << i)) {
                vesa_set_pixel(x + 8 - i, y + j, color);
            } else {
                vesa_set_pixel(x + 8 - i, y + j, 0);
            }
        }
    }
}

void tef_redraw() {
    for (int i = 0; i < MAX_COLS; i++) {
        for (int j = 0; j < MAX_ROWS; j++) {
            tef_set_char(i * FONT_WIDTH, j * FONT_HEIGHT, char_buffer[i + j * MAX_COLS], color_buffer[i + j * MAX_COLS]);
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

    if (*x >= MAX_COLS) {
        *x = 0;
        *y += 1;
    }

    // scroll
    if (*y >= MAX_ROWS) {
        for (int i = 0; i < MAX_COLS * (MAX_ROWS - 1); i++) {
            char_buffer[i] = char_buffer[i + MAX_COLS];
            color_buffer[i] = color_buffer[i + MAX_COLS];
        }
        for (int i = MAX_COLS * (MAX_ROWS - 1); i < MAX_COLS * MAX_ROWS; i++) {
            char_buffer[i] = ' ';
            color_buffer[i] = color;
        }
        *y = MAX_ROWS - 1;
        tef_redraw();
    }

    // update cursor
    tef_set_char((*x) * FONT_WIDTH, (*y) * FONT_HEIGHT, '_', 0x00FF00);
}

void tef_print(char *message, int x, int y, uint32_t color) {
    // if cursor_x or cursor_y is -1, use the current cursor position
    for (int i = 0; message[i] != '\0'; i++) {
        if (x == -1 && y == -1) {
            tef_print_char(message[i], &cursor_x, &cursor_y, color);
        } else {
            tef_print_char(message[i], &x, &y, color);
        }
    }

}

void tef_clear() {
    for (int i = 0; i < MAX_COLS * MAX_ROWS; i++) {
        char_buffer[i] = ' ';
        color_buffer[i] = 0xFFFFFF;
    }
    cursor_x = 0;
    cursor_y = 0;
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 768; j++) {
            vesa_set_pixel(i, j, 0);
        }
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
