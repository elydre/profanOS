#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <mem.h>

#define FONT_TABLE font_get_8x16()
#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#define SCROLLED_LINES 5

#define MAX_COLS (1024 / FONT_WIDTH)
#define MAX_ROWS (768 / FONT_HEIGHT)

#define CURSOR_COLOR 0xFF003c

int cursor_x = 0;
int cursor_y = 0;

int hidden_cursor = 0;


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
                vesa_set_pixel(x + 7 - i, y + j, color);
            } else {
                vesa_set_pixel(x + 7 - i, y + j, bg_color);
            }
        }
    }
}

void tef_draw_cursor(uint32_t color) {
    if (hidden_cursor) color = 0;
    // cursor is a empty rectangle
    for (int i = 0; i < FONT_WIDTH; i++) {
        vesa_set_pixel(cursor_x * FONT_WIDTH + i, cursor_y * FONT_HEIGHT, color);
        vesa_set_pixel(cursor_x * FONT_WIDTH + i, cursor_y * FONT_HEIGHT + FONT_HEIGHT - 1, color);
    }
    for (int i = 0; i < FONT_HEIGHT; i++) {
        vesa_set_pixel(cursor_x * FONT_WIDTH, cursor_y * FONT_HEIGHT + i, color);
        vesa_set_pixel(cursor_x * FONT_WIDTH + FONT_WIDTH - 1, cursor_y * FONT_HEIGHT + i, color);
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
        tef_draw_cursor(0);
        cursor_x--;
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
        mem_move((uint8_t*)(fb + 1024 * 16 * SCROLLED_LINES), (uint8_t*) fb, 1024 * 16 * (768 - 16 * SCROLLED_LINES));
        // clear the last lines
        for (int i = 0; i < 1024; i++) {
            for (int j = 768 - 16 * SCROLLED_LINES; j < 768; j++) {
                vesa_set_pixel(i, j, 0);
            }
        }
        cursor_y -= SCROLLED_LINES;
        cursor_x = 0;
        return;
    }
}

void tef_print(char *message, int x, int y, uint32_t color, uint32_t bg_color) {
    // clean the actual cursor
    tef_draw_cursor(0);

    for (int i = 0; message[i] != '\0'; i++) {
        tef_print_char(message[i], x, y, color, bg_color);
        x = -1;
        y = -1;
    }

    // draw the cursor
    tef_draw_cursor(CURSOR_COLOR);
}


int tef_get_cursor_offset() {
    // we have to multiply by 2 for the text mode compatibility
    return (cursor_y * MAX_COLS + cursor_x) * 2;
}

void tef_set_cursor_offset(int offset) {
    // clean the actual cursor
    tef_draw_cursor(0);
    
    // we have to divide by 2 for the text mode compatibility
    offset /= 2;
    cursor_y = offset / MAX_COLS;
    cursor_x = offset % MAX_COLS;

    // draw the cursor
    tef_draw_cursor(CURSOR_COLOR);
}

void tef_cursor_blink(int off) {
    hidden_cursor = off;
    tef_draw_cursor(CURSOR_COLOR);
}
