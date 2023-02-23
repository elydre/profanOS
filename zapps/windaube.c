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

void perf_demo();
void main_process();

int main(int argc, char **argv) {
    c_run_ifexist("/bin/commands/cpu.bin", 0, NULL);

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
    desktop->windows[0] = window_create(desktop, "desktop", 1, 1, 1022, 766, 0, 0);
    desktop->windows[1] = window_create(desktop, "classic 2", 100, 200, 200, 200, 2, 0);
    desktop->windows[2] = window_create(desktop, "classic 3", 70, 70, 300, 300, 1, 0);
    desktop->windows[3] = window_create(desktop, "lite 1", 240, 240, 100, 100, 3, 1);
    desktop->mouse = mouse_create();
    desktop_refresh(desktop);

    window_fill(desktop->windows[3], 0x222222);
    window_refresh(desktop, desktop->windows[3]);

    int demo_pid = c_process_create(perf_demo, 5, "demo");
    c_process_wakeup(demo_pid);

    while (1) {
        refresh_mouse(desktop);
        ms_sleep(10);
    }
}

void perf_demo() {
    window_t *window = desktop->windows[1];
    // square that bounces on the edge of the window like pong

    int square_x = 10;
    int square_y = 0;

    int old_square_x = 0;
    int old_square_y = 0;

    float square_speed_x = 6;
    float square_speed_y = 4;

    window_fill(window, 0xff0000);
    while (1) {
        // c_serial_print(SERIAL_PORT_A, "tick\n");
        // draw the old square

        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                if (old_square_x + i < window->in_width && old_square_y + j < window->in_height && old_square_x + i >= 0 && old_square_y + j >= 0) {
                    window_set_pixel(window, old_square_x + i, old_square_y + j, 0xaa0000);
                }
            }
        }

        // draw the new square

        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                if (square_x + i < window->in_width && square_y + j < window->in_height && square_x + i >= 0 && square_y + j >= 0) {
                    window_set_pixel(window, square_x + i, square_y + j, 0xffffff);
                }
            }
        }

        window_refresh(desktop, window);

        old_square_x = square_x;
        old_square_y = square_y;

        square_x += square_speed_x;
        square_y += square_speed_y;

        if (square_x > window->in_width - 10) {
            square_speed_x = -5;
        }
        if (square_x < 0) {
            square_speed_x = 7;
        }
        if (square_y > window->in_height - 10) {
            square_speed_y = -3;
        }
        if (square_y < 0) {
            square_speed_y = 7;
        }
        ms_sleep(2);
    }
}
