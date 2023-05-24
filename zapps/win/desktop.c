#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_ocmlib.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    // get the main desktop
    desktop_t *main_desktop = desktop_get_main();

    // create a window and add an exit button
    window_t *window = window_create(main_desktop, "desktop", 1, 1, 1022, 766, 1, 1, 0);
    desktop_refresh(main_desktop);

    ocm_init(window);

    // printf("Hello, world!\n");

    while (1) {
        ms_sleep(200);
    }

    return 0;
}
