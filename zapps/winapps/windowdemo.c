#include <i_libdaube.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>

int is_running = 1;

void callback(clickevent_t *event) {
    is_running = 0;
    window_delete(((button_t *) event->button)->window);
}

void draw_exit_button(window_t *window) {
    for (int i = 1; i < 19; i++) {
        for (int j = 1; j < 19; j++) {
            window_set_pixel_out(window, 111 - i, j, 0xff0000);
        }
    }
}

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    window_t *window = window_create(desktop_get_main(), "demo", 400, 200, 100, 100, 0, 0);
    button_t *button = create_button(window, 1, 1, 19, 19, callback);
    draw_exit_button(window);
    desktop_refresh(desktop_get_main());

    // reset pixel buffer
    window_fill(window, 0x000000);
    window_refresh(window);

    while (is_running) {
        ms_sleep(200);
    }
    return 0;
}
