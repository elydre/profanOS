#include "syscall.h"

#define SLEEP_TIME 3500

int main(int arg) {
    c_vga_320_mode();
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 25*40; j++) {
            c_vga_set_pixel(i%8 * 40 + j%40, i/8 * 25 + j/40, i);
        }
    }
    c_vga_print(45, 55, "320x200 - 64 colors (fast)", 1, 0);
    c_ms_sleep(SLEEP_TIME);
    c_vga_clear_screen();
    c_vga_print(40, 40, "little font", 0, 0);
    c_vga_print(40, 60, "big font", 1, 0);
    c_vga_print(40, 80, "colored print", 1, 4);
    c_ms_sleep(SLEEP_TIME);
    c_vga_clear_screen();
    for (int i = 0; i < 0x500; i++) {
        c_vga_draw_line(c_rand() % c_vga_get_width(),
                          c_rand() % c_vga_get_height(),
                          c_rand() % c_vga_get_width(),
                          c_rand() % c_vga_get_height(),
                          c_rand() % 64);
    }
    c_vga_draw_rect(30, 30, 176, 35, 0);
    c_vga_draw_rect(32, 32, 172, 31, 63);
    c_vga_print(40, 40, "integrated function", 1, 0);
    c_ms_sleep(SLEEP_TIME);
    c_vga_640_mode();
    c_vga_print(40, 40, "640x480 - 16 colors (slow)", 1, 0);
    
    for (int j = 0; j < 400; j += 100) {
        for (int i = 0; i < 64; i++) {
        c_vga_draw_line(100 + i + j, 400, 200 + i + j, 200 - i, i % 16);
        }
    }

    c_ms_sleep(SLEEP_TIME);

    c_vga_text_mode();
    return arg;
}
