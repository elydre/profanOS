#include <i_libdaube.h>
#include <i_string.h>
#include <i_iolib.h>
#include <syscall.h>
#include <string.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>
#include <i_vgui.h>

desktop_t *desktop;

void main_process();

int main(int argc, char **argv) {
    
    // we run the process
    int pid1 = c_process_create(main_process, "windaube_main");
    // we awake them
    c_process_wakeup(pid1);

    while (1);
    return 0;
}

void main_process() {
    vgui_t vgui = vgui_setup(1024, 768);
    vgui_clear(&vgui, 0x000000);
    vgui_render(&vgui, 0);
    desktop = malloc(sizeof(desktop_t));
    desktop->nb_windows = 2;
    desktop->windows = malloc(sizeof(window_t *) * desktop->nb_windows);
    int i = 0;
    int j = 0;
    while (1) {
        vgui_clear(&vgui, 0x000000);
        desktop->windows[0] = window_create("test", 100, 100, 0, 0, 0);
        desktop->windows[1] = window_create("test2", 100, 100, i, j, 1);

        desktop_draw(&vgui, desktop);

        i++;j++;
    }

}
