#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_time.h>

#include <syscall.h>

int is_running;

void exit_callback(clickevent_t *event) {
    is_running = 0;
    window_delete(((button_t *) event->button)->window);
}

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    window_t *window = window_create(desktop_get_main(), "pong like", 100, 200, 200, 200, 0, 0);
    wadds_create_exitbt(window, exit_callback);

    desktop_refresh(desktop_get_main());
    // square that bounces on the edge of the window like pong

    int square_x = 10;
    int square_y = 0;

    int old_square_x = 0;
    int old_square_y = 0;

    float square_speed_x = 6;
    float square_speed_y = 4;

    window_fill(window, 0xff0000);

    is_running = 1;
    while (is_running) {
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