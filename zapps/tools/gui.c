#include "syscall.h"

#define SLEEP_TIME 3500

int main(int argc, char **argv) {
    c_vga_320_mode();
    c_vgui_setup(0);
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 25*40; j++) {
            c_vgui_set_pixel(i%8 * 40 + j%40, i/8 * 25 + j/40, i);
        }
    }
    c_vgui_print(45, 55, "320x200 - 64 colors (fast)", 1, 0);
    c_vgui_render();
    c_ms_sleep(SLEEP_TIME);
    c_vgui_clear(0x0F);
    c_vgui_print(40, 40, "little font", 0, 0);
    c_vgui_print(40, 60, "big font", 1, 0);
    c_vgui_print(40, 80, "colored print", 1, 4);
    c_vgui_render();
    c_ms_sleep(SLEEP_TIME);
    c_vgui_clear(0);
    for (int i = 0; i < 0x500; i++) {
        c_vgui_draw_line(c_rand() % c_vga_get_width(),
                        c_rand() % c_vga_get_height(),
                        c_rand() % c_vga_get_width(),
                        c_rand() % c_vga_get_height(),
                        c_rand() % 64);
    }
    c_vgui_draw_rect(30, 30, 176, 35, 0);
    c_vgui_draw_rect(32, 32, 172, 31, 63);
    c_vgui_print(40, 40, "integrated function", 1, 0);
    c_vgui_render();
    c_ms_sleep(SLEEP_TIME);
    c_vgui_exit();
    c_vga_text_mode();
    return 0;
}
