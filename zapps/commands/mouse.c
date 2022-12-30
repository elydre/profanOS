#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <i_time.h>
#include <i_vgui.h>

int main(int argc, char **argv) {

    c_mouse_install();
    c_clear_screen();

    int x, y;

    while (1) {
        x = c_mouse_get_x();
        y = c_mouse_get_y();
        // // we draw a rectangle
        printf("x: %d, y: %d, %d %d %d\n", x, y, c_mouse_get_button(0), c_mouse_get_button(1), c_mouse_get_button(2));
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                c_vesa_set_pixel(x+i, y+j, 0x00FF00);
            }
        }
        ms_sleep(25);
    }
    return 0;
}