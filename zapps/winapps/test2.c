#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>

int is_running;

void exit_callback(clickevent_t *event) {
    is_running = 0;
}

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    // get the main desktop
    desktop_t *main_desktop = desktop_get_main();

    // create a window and add an exit button
    window_t *window = window_create(main_desktop, "demo", 250, 100, 100, 100, 0, 0);
    wadds_create_exitbt(window, exit_callback);
    desktop_refresh(main_desktop);

    // set the window background to black

    is_running = 1;
    while (is_running) {
        window_fill(window, 0xFF0000);
        window_refresh(window);

        window_fill(window, 0x00FF00);
        window_refresh(window);

        window_fill(window, 0x0000FF);
        window_refresh(window);
    }

    // destroy window and wait for it to be deleted
    window_delete(window);
    window_wait_delete(main_desktop, window);

    return 0;
}
