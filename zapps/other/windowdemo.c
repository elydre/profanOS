#include <i_libdaube.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>

void callback(clickevent_t *event) {
    window_delete(((button_t *) event->button)->window);
}

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    window_t *window = window_create(desktop_get_main(), "WINDOW DEMO", 400, 200, 100, 100, 0, 0);
    button_t *button = create_button(window, 0, 0, 20, 20, callback);
    desktop_refresh(desktop_get_main());

    while (1) {
        // reset pixel buffer
        window_fill(window, 0x000000);
        window_refresh(window);
        ms_sleep(200);
    }
}
