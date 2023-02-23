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
    desktop->windows[0] = window_create(desktop, "classic 1", 150, 150, 100, 100, 1, 0);
    desktop->windows[1] = window_create(desktop, "classic 2", 100, 200, 200, 200, 2, 0);
    desktop->windows[2] = window_create(desktop, "classic 3", 70, 70, 300, 300, 0, 0);
    desktop->windows[3] = window_create(desktop, "lite 1", 250, 250, 100, 100, 3, 1);
    desktop_refresh(desktop);

    window_fill(desktop->windows[1], 0x0000ff);
    window_refresh(desktop, desktop->windows[1]);

    while (1) {        
        ms_sleep(100);
    }
}
