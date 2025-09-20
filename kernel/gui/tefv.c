/*****************************************************************************\
|   === tefv.c : 2024 ===                                                     |
|                                                                             |
|    Kernel Text Mode Emulator for VESA                            .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <minilib.h>

#define FONT_TABLE font_8x16
#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#define SCROLLED_LINES 5

#define MAX_COLS ((int) vesa_get_width() / FONT_WIDTH)
#define MAX_ROWS ((int) vesa_get_height() / FONT_HEIGHT)

#define CURSOR_COLOR 0xFF003C

typedef struct {
    char content;
    char color;
} screen_char_t;

int cursor_x = 0;
int cursor_y = 0;

int hidden_cursor = 1;

screen_char_t *screen_buffer = NULL;

int tef_init(void) {
    if (!vesa_does_enable())
        return 2;
    screen_buffer = calloc(MAX_COLS * MAX_ROWS * sizeof(screen_char_t));
    return screen_buffer == NULL;
}

uint32_t compute_color(uint8_t color) {
    uint32_t rgb[] = {
        0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
        0xAA0000, 0xAA00AA, 0xAA5500, 0xBBBBBB,
        0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
        0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
    };
    if (color > 0xF) return 0xFFFFFF;
    return rgb[(int) color];
}

// a character at a given position
void tef_set_char(int x, int y, char c, char color) {
    if (x < 0 || x >= MAX_COLS || y < 0 || y >= MAX_ROWS) return;
    if (screen_buffer != NULL) {
        if (screen_buffer[y * MAX_COLS + x].content == c &&
            screen_buffer[y * MAX_COLS + x].color == color) {
            return;
        }
        screen_buffer[y * MAX_COLS + x].content = c;
        screen_buffer[y * MAX_COLS + x].color = color;
    }
    uint32_t bg_color = compute_color(color >> 4 & 0xF);
    uint32_t fg_color = compute_color(color & 0xF);
    int i, j;
    for (i = 0; i < FONT_WIDTH; i++) {
        for (j = 0; j < FONT_HEIGHT; j++) {
            if (FONT_TABLE[c * FONT_HEIGHT + j] & (1 << i)) {
                vesa_set_pixel((x + 1) * FONT_WIDTH - i, y * FONT_HEIGHT + j, fg_color);
            } else {
                vesa_set_pixel((x + 1) * FONT_WIDTH - i, y * FONT_HEIGHT + j, bg_color);
            }
        }
    }
}

void tef_draw_cursor(uint32_t color) {
    if (hidden_cursor) return;
    for (int i = -2; i < 3; i++) {
        if (cursor_x * FONT_WIDTH + i < 0 || cursor_x * FONT_WIDTH + i >= (int) vesa_get_width()) continue;
        vesa_set_pixel(cursor_x * FONT_WIDTH + i, cursor_y * FONT_HEIGHT, color);
        vesa_set_pixel(cursor_x * FONT_WIDTH + i, cursor_y * FONT_HEIGHT + FONT_HEIGHT - 1, color);
    }
    for (int i = 0; i < FONT_HEIGHT; i++) {
        if (cursor_y * FONT_HEIGHT + i < 0 || cursor_y * FONT_HEIGHT + i >= (int) vesa_get_height()) continue;
        vesa_set_pixel(cursor_x * FONT_WIDTH, cursor_y * FONT_HEIGHT + i, color);
    }
}

void tef_print_char(char c, char color) {
    tef_draw_cursor(0);

    if (c == '\n') {
        // fill the rest of the line with spaces
        for (int i = cursor_x; i < MAX_COLS; i++) {
            if (!screen_buffer || screen_buffer[cursor_y * MAX_COLS + i].content == '\0') {
                tef_set_char(i, cursor_y, ' ', color);
            }
        }
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        tef_set_char(cursor_x, cursor_y, ' ', color);
        cursor_x++;
    } else if (c == 0x08) {
        // str_backspace
        tef_draw_cursor(0);
        cursor_x--;
        tef_set_char(cursor_x, cursor_y, ' ', color);
    } else {
        tef_set_char(cursor_x, cursor_y, c, color);
        cursor_x++;
    }

    if (cursor_x >= MAX_COLS) {
        cursor_x = 0;
        cursor_y++;
    }

    // scroll
    if (cursor_y >= MAX_ROWS) {
        int index;
        for (int i = 0; i < MAX_ROWS - SCROLLED_LINES; i++) {
            for (int j = 0; j < MAX_COLS; j++) {
                index = (i + SCROLLED_LINES) * MAX_COLS + j;
                tef_set_char(j, i, screen_buffer[index].content, screen_buffer[index].color);
            }
        }
        for (int i = MAX_ROWS - SCROLLED_LINES; i < MAX_ROWS; i++) {
            for (int j = 0; j < MAX_COLS; j++) {
                tef_set_char(j, i, ' ', color);
            }
        }
        cursor_y = MAX_ROWS - SCROLLED_LINES;
    }
    tef_draw_cursor(CURSOR_COLOR);
}

int tef_cursor_get_offset(void) {
    // we have to multiply by 2 for the text mode compatibility
    return (cursor_y * MAX_COLS + cursor_x) * 2;
}

void tef_cursor_set_offset(int offset) {
    // clean the actual cursor
    tef_draw_cursor(0);

    // we have to divide by 2 for the text mode compatibility
    offset /= 2;
    cursor_y = offset / MAX_COLS;
    cursor_x = offset % MAX_COLS;

    // draw the cursor
    tef_draw_cursor(CURSOR_COLOR);
}

void tef_cursor_blink(int on) {
    tef_draw_cursor(on ? CURSOR_COLOR : 0);
    hidden_cursor = !on;
    tef_draw_cursor(on ? CURSOR_COLOR : 0);
}

void tef_clear(void) {
    tef_draw_cursor(0);
    cursor_x = 0;
    cursor_y = 0;
    for (int i = 0; i < MAX_ROWS * MAX_COLS; i++) {
        screen_buffer[i].content = ' ';
        screen_buffer[i].color = 0;
    }
    // set pixel to black
    mem_set(vesa_get_fb(), 0, vesa_get_height() * vesa_get_pitch() * sizeof(uint32_t));
}
