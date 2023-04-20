#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>

#define TASKBAR_HEIGHT 32
#define TASKBAR_COLOR 0x0c0f1d

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
    window_fill(window, TASKBAR_COLOR);
    window_refresh(window);

    i_time_t time;
    char *char_buffer = malloc(256);

    while (1) {
        // get the current time from rtc
        c_time_get(&time);
        // display the time
        sprintf_s(char_buffer, 256, "%02d:%02d:%02d", time.hours, time.minutes, time.seconds);
        wadds_puts(window, SCREEN_WIDTH - (8*8 + 5), 1, char_buffer, 0xffffff, TASKBAR_COLOR);
        // display the date
        sprintf_s(char_buffer, 256, "%02d/%02d/%02d", time.day_of_month, time.month, time.year);
        wadds_puts(window, SCREEN_WIDTH - (8*8 + 5), 17, char_buffer, 0xffffff, TASKBAR_COLOR);

        window_refresh(window);
        
        ms_sleep(250);
    }

    return 0;
}
