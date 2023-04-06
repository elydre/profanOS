#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>

int is_running;

void exit_callback(clickevent_t *event) {
    is_running = 0;
    window_delete(((button_t *) event->button)->window);
}

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    window_t *window = window_create(desktop_get_main(), "demo", 380, 220, 100, 100, 0, 0);
    wadds_create_exitbt(window, exit_callback);
    desktop_refresh(desktop_get_main());

    // reset pixel buffer
    window_fill(window, 0x000000);
    window_refresh(window);

    is_running = 1;
    while (is_running) {
        ms_sleep(200);
    }

    return 0;
}
