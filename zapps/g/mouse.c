/*****************************************************************************\
|   === mouse.c : 2024 ===                                                    |
|                                                                             |
|    Basic mouse input test                                        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/syscall.h>
#include <profan/mouse.h>
#include <profan/vgui.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

uint32_t *fb;
uint32_t pitch;
int width;
int height;

char data[21][12] = {
    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0},
    {1, 2, 2, 1, 1, 2, 2, 1, 0, 0, 0, 0},
    {1, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 0},
    {1, 1, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
};

uint32_t old[21][12];

void draw_mouse(int x, int y) {
    // draw the mouse
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 12; j++) {
            if (x + j < 0 || x + j >= width || y + i < 0 || y + i >= height)
                continue;
            switch (data[i][j]) {
                case 0:
                    break;
                case 1:
                    old[i][j] = fb[(y + i) * pitch + (x + j)];
                    fb[(y + i) * pitch + (x + j)] = 0xFFFFFF;
                    break;
                case 2:
                    old[i][j] = fb[(y + i) * pitch + (x + j)];
                    fb[(y + i) * pitch + (x + j)] = 0x333333;
                    break;
            }

        }
    }
}

void restore_mouse(int x, int y) {
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 12; j++) {
            if (x + j < 0 || x + j >= width || y + i < 0 || y + i >= height)
                continue;
            if (data[i][j] != 0)
                fb[(y + i) * pitch + (x + j)] = old[i][j];
        }
    }
}

int main(void) {
    int x, y;

    pitch = syscall_vesa_pitch();
    fb = syscall_vesa_fb();

    width = syscall_vesa_width();
    height = syscall_vesa_height();

    while (1) {
        x = mouse_get_x();
        y = mouse_get_y();
        draw_mouse(x, y);
        usleep(10000);
        restore_mouse(x, y);
    }
    return 0;
}
