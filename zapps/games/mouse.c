#include <syscall.h>
#include <i_mouse.h>
#include <stdlib.h>
#include <unistd.h>
#include <i_vgui.h>
#include <stdio.h>

void draw_mouse(int x, int y);

int main(int argc, char **argv) {
    printf("\e[2J");

    int x, y;

    while (1) {
        x = mouse_get_x();
        y = mouse_get_y();
        // // we draw a rectangle
        printf("x: %d, y: %d, %d %d %d\n", x, y, mouse_get_button(0), mouse_get_button(1), mouse_get_button(2));
        draw_mouse(x, y);
        usleep(25000);
    }
    return 0;
}

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

    // draw the mouse
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 12; j++) {
            switch (data[i][j]) {
                case 0:
                    break;
                case 1:
                    c_vesa_set_pixel(x + j, y + i, red);
                    break;
                case 2:
                    c_vesa_set_pixel(x + j, y + i, white);
                    break;
            }

        }
    }
}
