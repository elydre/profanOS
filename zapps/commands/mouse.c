#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <i_time.h>
#include <i_vgui.h>

int main(int argc, char **argv) {
    // c_mouse_install();
    c_clear_screen();
    vgui_draw_rect(50, 50, 10, 10, 0x00FF00);
    vgui_render();

    while (1) {
        // c_clear_screen();
        // printf("%d\n", c_mouse_get_x());
        // printf("%d\n", c_mouse_get_y());
        // printf("%d\n", c_mouse_read());
        // printf("%d\n", c_mouse_get_button(0));
        // printf("%d\n", c_mouse_get_button(1));
        // printf("%d\n", c_mouse_get_button(2));
        // // we draw a rectangle
        
        // ms_sleep(100);
    }
    return 0;
}