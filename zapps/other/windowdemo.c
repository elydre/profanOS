#include <i_libdaube.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    window_t *window = window_create(desktop_get_main(), "WINDOW DEMO", 500, 100, 100, 100, 0, 0);
    desktop_refresh(desktop_get_main());

    while (1) {
        // reset pixel buffer
        window_fill(window, 0x000000);
        window_refresh(window);
        ms_sleep(200);
    }
}
