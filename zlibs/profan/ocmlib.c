#include <syscall.h>
#include <stdlib.h>

#include <i_libdaube.h>

typedef struct {
    char content;
    uint32_t color;
    uint32_t bg_color;
} screen_char_t;

screen_char_t *screen_buffer;
window_t *term_window;

#define FONT_TABLE c_font_get(FONT_8X16)
#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#define SCROLLED_LINES 5

#define X_OFFSET 2
#define Y_OFFSET 2

#define FONT_8X16 0

#define MAX_COLS ((int) (c_vesa_get_width() - X_OFFSET * 2) / FONT_WIDTH)
#define MAX_ROWS ((int) (c_vesa_get_height() - Y_OFFSET * 2) / FONT_HEIGHT)

#define CURSOR_COLOR 0x00FF3C

int main(int argc, char **argv) {
    screen_buffer = NULL;
    term_window = NULL;

    return 0;
}

int cursor_x;
int cursor_y;

int hidden_cursor;

void ocm_init(window_t *window) {
    screen_buffer = calloc(MAX_COLS * MAX_ROWS, sizeof(screen_char_t));
    for (int i = 0; i < MAX_COLS * MAX_ROWS; i++) {
        screen_buffer[i].content = ' ';
        screen_buffer[i].color = 0;
        screen_buffer[i].bg_color = 0;
    }

    cursor_x = 0;
    cursor_y = 0;
    hidden_cursor = 1;

    term_window = window;
}


// set a character at a given position
void set_char(int x, int y, char c, uint32_t color, uint32_t bg_color) {
    if (x < 0 || x >= MAX_COLS || y < 0 || y >= MAX_ROWS) return;
    if (screen_buffer != NULL) {
        if (screen_buffer[y * MAX_COLS + x].content == c &&
            screen_buffer[y * MAX_COLS + x].color == color &&
            screen_buffer[y * MAX_COLS + x].bg_color == bg_color) {
            return;
        }
        screen_buffer[y * MAX_COLS + x].content = c;
        screen_buffer[y * MAX_COLS + x].color = color;
        screen_buffer[y * MAX_COLS + x].bg_color = bg_color;
    }
    int i, j;
    for (i = 0; i < FONT_WIDTH; i++) {
        for (j = 0; j < FONT_HEIGHT; j++) {
            if (FONT_TABLE[c * FONT_HEIGHT + j] & (1 << i)) {
                window_display_pixel(term_window, X_OFFSET + (x + 1) * FONT_WIDTH - i, Y_OFFSET + y * FONT_HEIGHT + j, color);
            } else {
                window_display_pixel(term_window, X_OFFSET + (x + 1) * FONT_WIDTH - i, Y_OFFSET + y * FONT_HEIGHT + j, bg_color);
            }
        }
    }
}

void draw_cursor(uint32_t color) {
    if (hidden_cursor) color = 0;
    for (int i = -2; i < 3; i++) {
        if (cursor_x * FONT_WIDTH + i < 0 || cursor_x * FONT_WIDTH + i >= term_window->in_width - X_OFFSET * 2) continue;
        window_display_pixel(term_window, X_OFFSET + cursor_x * FONT_WIDTH + i, Y_OFFSET + cursor_y * FONT_HEIGHT, color);
        window_display_pixel(term_window, X_OFFSET + cursor_x * FONT_WIDTH + i, Y_OFFSET + cursor_y * FONT_HEIGHT + FONT_HEIGHT - 1, color);
    }
    for (int i = 0; i < FONT_HEIGHT; i++) {
        if (cursor_y * FONT_HEIGHT + i < 0 || cursor_y * FONT_HEIGHT + i >= term_window->in_height - Y_OFFSET * 2) continue;
        window_display_pixel(term_window, X_OFFSET + cursor_x * FONT_WIDTH, Y_OFFSET + cursor_y * FONT_HEIGHT + i, color);
    }
}

void print_char(char c, int x, int y, uint32_t color, uint32_t bg_color) {
    if (x != -1) cursor_x = x;
    if (y != -1) cursor_y = y;

    if (c == '\n') {
        // fill the rest of the line with spaces
        for (int i = cursor_x; i < MAX_COLS; i++) {
            if (screen_buffer[cursor_y * MAX_COLS + i].content == '\0') {
                set_char(i, cursor_y, ' ', color, bg_color);
            }
        }
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\a') {
        // str_bell
    } else if (c == '\t') {
        set_char(cursor_x, cursor_y, ' ', color, bg_color);
        cursor_x++;
    } else if (c == 0x08) {
        // str_backspace
        draw_cursor(0);
        cursor_x--;
        set_char(cursor_x, cursor_y, ' ', color, bg_color);
    } else {
        set_char(cursor_x, cursor_y, c, color, bg_color);
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
                set_char(j, i, screen_buffer[index].content, screen_buffer[index].color, screen_buffer[index].bg_color);
            }
        }
        for (int i = MAX_ROWS - SCROLLED_LINES; i < MAX_ROWS; i++) {
            for (int j = 0; j < MAX_COLS; j++) {
                set_char(j, i, ' ', 0, 0);
            }
        }
        cursor_y = MAX_ROWS - SCROLLED_LINES;
    }
}

void ocm_print(char *message, int x, int y, uint32_t color, uint32_t bg_color) {
    if (screen_buffer == NULL || term_window == NULL) return;
    // clean the actual cursor
    draw_cursor(0);

    for (int i = 0; message[i] != '\0'; i++) {
        print_char(message[i], x, y, color, bg_color);
        x = -1;
        y = -1;
    }

    // draw the cursor
    draw_cursor(CURSOR_COLOR);
}

int ocm_get_cursor_offset() {
    // we have to multiply by 2 for the text mode compatibility
    return (cursor_y * MAX_COLS + cursor_x) * 2;
}

void ocm_set_cursor_offset(int offset) {
    if (screen_buffer == NULL || term_window == NULL) return;
    // clean the actual cursor
    draw_cursor(0);
    
    // we have to divide by 2 for the text mode compatibility
    offset /= 2;
    cursor_y = offset / MAX_COLS;
    cursor_x = offset % MAX_COLS;

    // draw the cursor
    draw_cursor(CURSOR_COLOR);
}

void ocm_cursor_blink(int on) {
    hidden_cursor = !on;
    draw_cursor(CURSOR_COLOR);
}

void ocm_clear() {
    if (screen_buffer == NULL || term_window == NULL) return;
    draw_cursor(0);
    cursor_x = 0;
    cursor_y = 0;

    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS; j++) {
            set_char(j, i, ' ', 0, 0);
        }
    }
}

int ocm_get_max_rows() {
    return MAX_ROWS;
}

int ocm_get_max_cols() {
    return MAX_COLS;
}
