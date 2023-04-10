#include <i_libdaube.h>
#include <i_time.h>

#include <syscall.h>

#define MAX_WINDOWS 20

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

void start_addons();

int main(int argc, char **argv) {
    desktop_t *desktop = desktop_init(MAX_WINDOWS, SCREEN_WIDTH, SCREEN_HEIGHT);

    c_run_ifexist("/bin/winapps/cpu.bin", 0, NULL);
    c_run_ifexist("/bin/winapps/demo.bin", 0, NULL);
    c_run_ifexist("/bin/winapps/pong.bin", 0, NULL);
    c_run_ifexist("/bin/winapps/counter.bin", 0, NULL);
    c_run_ifexist("/bin/winapps/usage.bin", 0, NULL);
    c_run_ifexist("/bin/winapps/term.bin", 0, NULL);
    // c_run_ifexist("/bin/winapps/cube.bin", 0, NULL);

    // c_run_ifexist("/bin/winapps/test1.bin", 0, NULL);
    // c_run_ifexist("/bin/winapps/test2.bin", 0, NULL);

    while (1) {
        // run the functions stack
        desktop_run_stack(desktop);

        refresh_mouse(desktop);

        start_addons(desktop);
        ms_sleep(10);
    }

    return 0;
}

void start_addons(desktop_t *desktop) {
    // fill screen if F7 is pressed
    if (c_kb_get_scancode() == 65) {
        for (int i = 0; i < desktop->screen_width; i++) {
            for (int j = 0; j < desktop->screen_height; j++) {
                c_vesa_set_pixel(i, j, (0x010101 * (i + j)) % 0xFFFFFF);
            }
        }
    }
    // restore screen if F8 is pressed
    if (c_kb_get_scancode() == 66) {
        for (int i = 0; i < desktop->screen_width; i++) {
            for (int j = 0; j < desktop->screen_height; j++) {
                c_vesa_set_pixel(i, j, desktop->screen_buffer[i + j * desktop->screen_width]);
            }
        }
    }
}
