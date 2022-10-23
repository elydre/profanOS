#include "syscall.h"


int main(int argc, char **argv) {
    c_vga_320_mode();
    c_vgui_setup(0);
    int color = 0;
    while (c_kb_get_scancode() != 1) {
        color = (color + 1) % 64;
        c_vgui_draw_rect(10, 10, 100, 100, color);
        c_vgui_render();
    }

    c_vgui_exit();
    c_vga_text_mode();

    c_fskprint("exit\n");

    return 0;
}
