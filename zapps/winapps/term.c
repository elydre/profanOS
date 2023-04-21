#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_ocmlib.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>

char msg[] = "Hello, world!";
int is_running;

#define WINW_HEIGHT 208
#define FONT_HEIGHT 16
#define PRT_LINES (WINW_HEIGHT / FONT_HEIGHT)

#define MONITORED_OCM 0

// we dont have virtual memory yet
uint32_t color_code_convert(char code) {
    switch (code) {
        case '0': return 0x9999ff;
        case '1': return 0x99ff99;
        case '2': return 0x99ffff;
        case '3': return 0xff9999;
        case '4': return 0xff99ff;
        case '5': return 0xffff99;
        case '6': return 0xaaaaaa;
        case '7': return 0xffffff;
        case '8': return 0x000099;
        case '9': return 0x009900;
        case 'A': return 0x009999;
        case 'B': return 0x990000;
        case 'C': return 0x990099;
        case 'D': return 0x999900;
        case 'E': return 0x999999;
        case 'F': return 0x000000;
    }
    return 0;
}

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

void print_from_ocm(window_t *window) {
    // get from the buffer the last PRT_LINES lines
    int len = ocm_get_len(MONITORED_OCM);
    int start = 0;
    int count = 0;
    for (int i = len - 1; i >= 0; i--) {
#include <i_winadds.h>
        if (ocm_read(MONITORED_OCM, i) == '\n') {
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
    int color = 0xffffff;
    for (int i = start; i < len; i++) {
        if (ocm_read(MONITORED_OCM, i) == '\n') {
            x = 0;
            y += FONT_HEIGHT;
        } else if (ocm_read(MONITORED_OCM, i) == '$') {
            color = color_code_convert(ocm_read(MONITORED_OCM, i + 1));
            i++;
        } else {
            local_print_char(window, ocm_read(MONITORED_OCM, i), x, y, color, 0);
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
    window_t *window = window_create(main_desktop, "gui term", 500, 450, 208 * 2, 208, 0, 0, 0);
    wadds_create_exitbt(window, exit_callback);
    desktop_refresh(main_desktop);

    is_running = 1;
    int last_refresh, last_update = 0;
    while (is_running) {
        // check if the terminal has been updated
        last_update = ocm_get_last_update(MONITORED_OCM);
        if (last_update == last_refresh) {
            ms_sleep(50);
            continue;
        }

        // refresh the window
        last_refresh = last_update;
        wadds_fill(window, 0x000000);

        print_from_ocm(window);

        window_refresh(window);
    }

    // destroy window and wait for it to be deleted
    window_delete(window);
    window_wait_delete(main_desktop, window);

    return 0;
}
