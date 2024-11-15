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

void draw_mouse(int x, int y) {
    int red = 0xFF0000;
    int white = 0xFFFFFF;

    int data[21][12] = {
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

    uint32_t pitch = syscall_vesa_pitch();
    uint32_t *fb = syscall_vesa_fb();

    // draw the mouse
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 12; j++) {
            switch (data[i][j]) {
                case 0:
                    break;
                case 1:
                    fb[(y + i) * pitch + (x + j)] = red;
                    break;
                case 2:
                    fb[(y + i) * pitch + (x + j)] = white;
                    break;
            }

        }
    }
}

int main(void) {
    printf("\e[2J");

    int x, y;

    while (1) {
        x = mouse_get_x();
        y = mouse_get_y();
        // we draw a rectangle
        printf("x: %d, y: %d, %d %d %d\n", x, y, mouse_get_button(0), mouse_get_button(1), mouse_get_button(2));
        draw_mouse(x, y);
        usleep(25000);
    }
    return 0;
}
