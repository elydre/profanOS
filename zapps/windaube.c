#include <i_libdaube.h>
#include <i_string.h>
#include <i_mouse.h>
#include <i_time.h>
#include <i_vgui.h>

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_WINDOWS 10

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

desktop_t *desktop;

void perf_demo();
void main_process();

int main(int argc, char **argv) {
    vgui_t vgui = vgui_setup(SCREEN_WIDTH, SCREEN_HEIGHT);

    desktop = desktop_init(&vgui, MAX_WINDOWS, SCREEN_WIDTH, SCREEN_HEIGHT);

    window_t *back = window_create(desktop, "desktop", 1, 1, 1022, 766, 0, 1, 1);
    window_t *lite = window_create(desktop, "lite 1", 240, 240, 100, 100, 3, 1, 0);

    desktop_refresh(desktop);

    window_fill(lite, 0x222222);
    window_refresh(lite);

    c_run_ifexist("/bin/commands/cpu.bin", 0, NULL);
    c_run_ifexist("/bin/other/windowdemo.bin", 0, NULL);

    int demo_pid = c_process_create(perf_demo, 1, "demo");
    c_process_wakeup(demo_pid);

    while (1) {
        refresh_mouse(desktop);
        ms_sleep(10);
    }
    return 0;
}

void perf_demo() {
    window_t *window = window_create(desktop, "pong like", 100, 200, 200, 200, 1, 0, 0);
    desktop_refresh(desktop);
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

        window_refresh(window);

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
        ms_sleep(50);
    }
}
