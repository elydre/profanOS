#include <profan/syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

uint32_t *screen;

#define WIDTH 640
#define HEIGHT 480

#define esc

void draw_cursor(int x, int y) {
    int i;
    for (i = -4; i < 5; i++) {
        if (x + i < 0 || x + i >= WIDTH) continue;
        c_vesa_set_pixel(x + i, y, 0xffffff);
    }
    for (i = -4; i < 5; i++) {
        if (y + i < 0 || y + i >= HEIGHT) continue;
        c_vesa_set_pixel(x, y + i, 0xffffff);
    }
}

void clear_cursor(int x, int y) {
    int i;
    for (i = -4; i < 5; i++) {
        if (x + i < 0 || x + i >= WIDTH) continue;
        c_vesa_set_pixel(x + i, y, screen[y * WIDTH + x + i]);
    }
    for (i = -4; i < 5; i++) {
        if (y + i < 0 || y + i >= HEIGHT) continue;
        c_vesa_set_pixel(x, y + i, screen[(y + i) * WIDTH + x]);
    }
}

void draw_col(uint32_t col) {
    for (int i = 0; i < WIDTH; i++) {
        c_vesa_set_pixel(i, 0, col);
        screen[i] = col;
    }
}

int main(void) {
    uint8_t keys[5];
    int k;

    uint32_t colors[] = {
        0x0000ff,
        0x00ff00,
        0x00ffff,
        0xff0000,
        0xff00ff,
        0xffff00,
        0xffffff,
    };
    int color = 6;

    int cursor_x = 50;
    int cursor_y = 50;

    int old_x, old_y;

    memset(keys, 0, sizeof(keys));

    screen = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            c_vesa_set_pixel(x, y, 0);
        }
    }

    draw_cursor(cursor_x, cursor_y);
    draw_col(colors[color]);

    while (1) {
        k = c_kb_get_scfh();
        if (k == KB_ESC) break;
        if (k % 128 == KB_LEFT) keys[0] = !(k / 128);
        if (k % 128 == KB_RIGHT) keys[1] = !(k / 128);
        if (k % 128 == KB_OLDER) keys[2] = !(k / 128);
        if (k % 128 == KB_NEWER) keys[3] = !(k / 128);
        if (k % 128 == KB_CTRL) keys[4] = !(k / 128);

        if (k % 128 == KB_A && !(k / 128)) {
            color = (color + 1) % (sizeof(colors) / sizeof(uint32_t));
            draw_col(colors[color]);
        }

        old_x = cursor_x;
        old_y = cursor_y;

        if (keys[0]) cursor_x--;
        if (keys[1]) cursor_x++;
        if (keys[2]) cursor_y--;
        if (keys[3]) cursor_y++;

        if (cursor_x < 0) cursor_x = 0;
        if (cursor_x >= WIDTH) cursor_x = WIDTH - 1;
        if (cursor_y < 0) cursor_y = 0;
        if (cursor_y >= HEIGHT) cursor_y = HEIGHT - 1;

        if (old_x != cursor_x || old_y != cursor_y) {
            clear_cursor(old_x, old_y);
            if (keys[4]) {
                c_vesa_set_pixel(cursor_x, cursor_y, colors[color]);
                screen[cursor_y * WIDTH + cursor_x] = colors[color];
            }
            draw_cursor(cursor_x, cursor_y);
        }
        usleep(10000);
    }

    free(screen);

    return 0;
}
