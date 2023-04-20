#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>

#define TASKBAR_HEIGHT 32

#define SCREEN_WIDTH c_vesa_get_width()
#define SCREEN_HEIGHT c_vesa_get_height()

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    // get the main desktop
    desktop_t *main_desktop = desktop_get_main();

    // create a window and add an exit button
    window_t *window = window_create(main_desktop, "taskbar", 1, SCREEN_HEIGHT - (TASKBAR_HEIGHT + 1), SCREEN_WIDTH - 2, TASKBAR_HEIGHT, 1, 1, 1);
    desktop_refresh(main_desktop);

    // set the window background
    window_fill(window, 0x0c0f1d);
    window_refresh(window);

    while (1) {
        ms_sleep(200);
    }

    return 0;
}
