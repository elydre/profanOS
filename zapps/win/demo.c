#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>


int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    // get the main desktop
    desktop_t *main_desktop = desktop_get_main();

    // create a window
    window_t *window = window_create(main_desktop, "on top", 380, 220, 100, 100, 0, 0, 1);
    desktop_refresh(main_desktop);

    // set the window background to black
    wadds_fill(window, 0x000000, 0);
    window_refresh(window);

    while (1) {
        ms_sleep(200);
    }

    // destroy window and wait for it to be deleted
    window_delete(window);
    window_wait_delete(main_desktop, window);

    return 0;
}
