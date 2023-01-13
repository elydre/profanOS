#include <i_libdaube.h>
#include <i_string.h>
#include <i_iolib.h>
#include <syscall.h>
#include <string.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>
#include <i_vgui.h>

int main(int argc, char **argv) {
    vgui_t vgui = vgui_setup(1024, 768);
    vgui_clear(&vgui, 0x000000);
    vgui_render(&vgui, 0);

    window_t *window = window_create("test", 100, 100, 0, 0);
    window_draw(&vgui, window);
    vgui_render(&vgui, 0);
    
    vgui_exit(&vgui);
    return 0;
}