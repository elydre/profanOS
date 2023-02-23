#include <i_libdaube.h>
#include <i_string.h>
#include <i_mouse.h>
#include <i_time.h>
#include <i_vgui.h>

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_WINDOWS 10

desktop_t *desktop;

void main_process();

int main(int argc, char **argv) {
    c_run_ifexist("/bin/commands/cpu.bin", 0, NULL);
    /*
    int pid1 = c_process_create(main_process, "windaube_main");
    c_process_wakeup(pid1);
    while (1);*/
    main_process();
    return 0;
}

void main_process() {
    vgui_t vgui = vgui_setup(1024, 768);
    vgui_clear(&vgui, 0x000000);
    vgui_render(&vgui, 0);
    desktop = malloc(sizeof(desktop_t));
    desktop->nb_windows = 0;
    desktop->vgui = &vgui;
    desktop->windows = malloc(sizeof(window_t *) * MAX_WINDOWS);
    desktop->windows[0] = window_create(desktop, "classic 1", 100, 100, 100, 100, 1);
    desktop->windows[1] = window_create(desktop, "classic 2", 50, 150, 200, 200, 2);
    desktop->windows[2] = window_create(desktop, "classic 3", 20, 20, 300, 300, 0);
    desktop->windows[3] = window_create(desktop, "lite 1", 200, 200, 100, 100, 3);
    desktop->windows[3]->is_lite = 1;
    desktop_refresh(desktop);

    int i = 100;
    int j = 100;
    while (1) {
        // window_move(desktop->windows[0], mouse_get_x(), mouse_get_y());
        // desktop_refresh(desktop);
        ms_sleep(100);
    }
}
