#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>

char msg[] = "Hello, world!";
int is_running;

#define WINW_HEIGHT 208
#define FONT_HEIGHT 16
#define PRT_LINES (WINW_HEIGHT / FONT_HEIGHT)

void exit_callback(clickevent_t *event) {
    is_running = 0;
}

void local_print_char(window_t *window, char c, int x, int y, uint32_t color, uint32_t bg_color) {
    uint8_t *glyph = c_font_get(0) + c * 16;
    for (int j = 0; j < 16; j++) {
        for (int k = 0; k < 8; k++) {
            window_set_pixel(window, x + 8 - k, y + j, (glyph[j] & (1 << k)) ? color : bg_color);
        }
    }
}

void print_from_buffer(window_t *window, char *buffer, int len) {
    // get from the buffer the last PRT_LINES lines
    int start = 0;
    int count = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (buffer[i] == '\n') {
            start = i + 1;
            count++;
        }
        if (count == PRT_LINES) {
            break;
        }
    }

    // print the lines
    int x = 0;
    int y = 0;
    for (int i = start; i < len; i++) {
        if (buffer[i] == '\n') {
            x = 0;
            y += FONT_HEIGHT;
        } else {
            local_print_char(window, buffer[i], x, y, 0xffffff, 0x660066);
            x += 8;
        }
    }
}

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    // get the main desktop
    desktop_t *main_desktop = desktop_get_main();

    // create a window and add an exit button
    window_t *window = window_create(main_desktop, "gui term", 500, 450, 208 * 2, 208, 0, 0);
    wadds_create_exitbt(window, exit_callback);
    desktop_refresh(main_desktop);

    is_running = 1;
    while (is_running) {
        // reset pixel buffer
        window_fill(window, 0x000000);

        print_from_buffer(window, wterm_get_buffer(), wterm_get_len());

        window_refresh(window);
        ms_sleep(50);
    }

    // destroy window and wait for it to be deleted
    window_delete(window);
    window_wait_delete(main_desktop, window);

    return 0;
}
