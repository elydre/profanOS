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

    // create a window and add an exit button
    window_t *window = window_create(main_desktop, "magnifier", 380, 220, 200, 200, 0, 0, 1);
    button_t *exit_button = wadds_create_exitbt(window);
    desktop_refresh(main_desktop);

    // set the window background to black
    wadds_fill(window, 0x000000, 0);

    while (!exit_button->clicked_tick) {
        for (int y = -window->height / 2; y < window->height / 2; y++) {
            for (int x = -window->width / 2; x < window->width / 2; x++) {
                if (main_desktop->mouse->x + x / 2 < 0 || main_desktop->mouse->x + x / 2 >= main_desktop->screen_width || main_desktop->mouse->y + y / 2 < 0 || main_desktop->mouse->y + y / 2 >= main_desktop->screen_height) {
                    window_set_pixel(window, x + window->width / 2, y + window->height / 2, 0x888888);
                    continue;
                };
                window_set_pixel(window, x + window->width / 2, y + window->height / 2, main_desktop->screen_buffer[(main_desktop->mouse->y + y / 2) * main_desktop->screen_width + (main_desktop->mouse->x + x / 2)]);
            }
        }
        window_refresh(window);
    }

    // destroy window and wait for it to be deleted
    window_delete(window);
    window_wait_delete(main_desktop, window);

    return 0;
}
