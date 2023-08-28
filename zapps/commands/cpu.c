#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>

#define HISTOTY_SIZE 100

int main(void) {
    int x_offset = c_vesa_get_width() - 1;
    if (x_offset < 100) {
        printf("[cpu] fail to start: screen too small\n");
        return 1;
    }

    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    int *history = calloc(HISTOTY_SIZE, sizeof(int));

    int cpu, last_idle, last_total;
    int idle = 0;
    int total = 0;

    uint32_t *pixel_buffer = calloc(100 * 100, sizeof(uint32_t));

    while (1) {
        last_idle = idle;
        last_total = total;

        idle = c_process_get_run_time(1);
        total = c_timer_get_ms();

        cpu = 100 - (idle - last_idle) * 100 / (total - last_total);

        for (int i = 0; i < HISTOTY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history[HISTOTY_SIZE - 1] = cpu;

        // reset pixel buffer
        for (int i = 0; i < 100 * 100; i++) {
            pixel_buffer[i] = 0x000000;
        }

        for (int i = 0; i < HISTOTY_SIZE; i++) {
            for (int j = 0; j < history[i]; j++) {
                pixel_buffer[i + 100 * j] = 0x00FFFF;
            }
        }

        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                c_vesa_set_pixel(x_offset - i, 100 - j, pixel_buffer[i + 100 * j]);
            }
        }
        c_process_sleep(c_process_get_pid(), 200);
    }
}
