#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>

#define TASKBAR_HEIGHT 32
#define TASKBAR_COLOR 0x0c0f1d

#define startMENU_WIDTH 150
#define startMENU_HEIGHT 200

#define SCREEN_WIDTH c_vesa_get_width()
#define SCREEN_HEIGHT c_vesa_get_height()

desktop_t *main_desktop;
window_t *start_menu;


void refresh_time(window_t *window, i_time_t time, char *char_buffer) {
    // display the time
    sprintf_s(char_buffer, 256, "%02d:%02d:%02d", time.hours, time.minutes, time.seconds);
    wadds_puts(window, char_buffer, SCREEN_WIDTH - (8*8 + 5), 1, 0xffffff, TASKBAR_COLOR);
    // display the date
    sprintf_s(char_buffer, 256, "%02d/%02d/%02d", time.day_of_month, time.month, time.year);
    wadds_puts(window, char_buffer, SCREEN_WIDTH - (8*8 + 5), 17, 0xffffff, TASKBAR_COLOR);    
}

window_t *create_start_menu() {
    window_t *window = window_create(main_desktop, "start menu", 1, SCREEN_HEIGHT - (TASKBAR_HEIGHT + startMENU_HEIGHT + 2), startMENU_WIDTH, startMENU_HEIGHT, 1, 1, 1);
    desktop_refresh(main_desktop);

    wadds_fill(window, TASKBAR_COLOR);

    window_refresh(window);

    return window;
}

int open_button_interact(button_t *button, int menu_button_tick) {
    int new_button_tick = button->clicked_tick;
    int button_clicked = new_button_tick != menu_button_tick;

    static int menu_open = 0;

    if (button_clicked) {
        if (menu_open == 0) {
            start_menu = create_start_menu();
            menu_open = 1;
        } else if (menu_open == 2 && start_menu != NULL) {
            window_delete(start_menu);
            menu_open = 0;
        }
    } else {
        if (menu_open == 1) {
            menu_open = 2;
        }
        if (menu_open == 3) {
            menu_open = 0;
        }
    }

    return new_button_tick;
}

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    // get the main desktop
    main_desktop = desktop_get_main();

    // create a window and add an exit button
    window_t *window = window_create(main_desktop, "taskbar", 1, SCREEN_HEIGHT - (TASKBAR_HEIGHT + 1), SCREEN_WIDTH - 2, TASKBAR_HEIGHT, 1, 1, 1);
    desktop_refresh(main_desktop);

    // set the window background
    wadds_fill(window, TASKBAR_COLOR);

    // set profanOS button
    button_t *menu_button = create_button(window, 1, 1, TASKBAR_HEIGHT, TASKBAR_HEIGHT);
    wadds_draw_bmp(window, "/zada/bmp/profan_logo.bmp", 1, 1);

    window_refresh(window);

    char *char_buffer = malloc(256);
    int need_refresh = 0;
    int old_seconds = 61;
    i_time_t time;

    int menu_button_tick = 0;
    start_menu = NULL;

    while (1) {
        // get the current time from rtc
        c_time_get(&time);
        if (time.seconds != old_seconds) {
            // refresh the time
            refresh_time(window, time, char_buffer);
            old_seconds = time.seconds;
            need_refresh = 1;
        }

        menu_button_tick = open_button_interact(menu_button, menu_button_tick);

        if (need_refresh) {
            window_refresh(window);
            need_refresh = 0;
        }

        ms_sleep(100);
    }

    return 0;
}
