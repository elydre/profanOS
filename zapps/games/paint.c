/*****************************************************************************\
|   === paint.c : 2024 ===                                                    |
|                                                                             |
|    Simple paint program                                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

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
    uint32_t pitch = syscall_vesa_pitch();
    uint32_t *fb = syscall_vesa_fb();

    int i;
    for (i = -4; i < 5; i++) {
        if (x + i < 0 || x + i >= WIDTH) continue;
        fb[y * pitch + x + i] = 0xffffff;
    }
    for (i = -4; i < 5; i++) {
        if (y + i < 0 || y + i >= HEIGHT) continue;
        fb[(y + i) * pitch + x] = 0xffffff;
    }
}

void clear_cursor(int x, int y) {
    uint32_t pitch = syscall_vesa_pitch();
    uint32_t *fb = syscall_vesa_fb();

    int i;
    for (i = -4; i < 5; i++) {
        if (x + i < 0 || x + i >= WIDTH) continue;
        fb[y * pitch + x + i] = screen[y * WIDTH + x + i];
    }
    for (i = -4; i < 5; i++) {
        if (y + i < 0 || y + i >= HEIGHT) continue;
        fb[(y + i) * pitch + x] = screen[(y + i) * WIDTH + x];
    }
}

void draw_col(uint32_t col) {
    uint32_t *fb = syscall_vesa_fb();

    for (int i = 0; i < WIDTH; i++) {
        fb[i] = col;
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

    uint32_t *fb = syscall_vesa_fb();
    uint32_t pitch = syscall_vesa_pitch();

    memset(keys, 0, sizeof(keys));

    screen = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            fb[y * pitch + x] = 0;
        }
    }

    draw_cursor(cursor_x, cursor_y);
    draw_col(colors[color]);

    while (1) {
        k = syscall_sc_get();
        if (k == KB_ESC) break;
        if (k % 128 == KB_LEFT) keys[0] = !(k / 128);
        if (k % 128 == KB_RIGHT) keys[1] = !(k / 128);
        if (k % 128 == KB_TOP) keys[2] = !(k / 128);
        if (k % 128 == KB_BOT) keys[3] = !(k / 128);
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
                screen[cursor_y * WIDTH + cursor_x] = colors[color];
                fb[cursor_y * pitch + cursor_x] = colors[color];
            }
            draw_cursor(cursor_x, cursor_y);
        }
        usleep(10000);
    }

    free(screen);

    return 0;
}
