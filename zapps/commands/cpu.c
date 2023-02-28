#include <i_libdaube.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    c_serial_print(SERIAL_PORT_A, "cpu usage\n");

    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    int history_size = 100;

    int *history = calloc(history_size, sizeof(int));

    int cpu, last_idle, last_total;
    int idle = 0;
    int total = 0;

    window_t *window = window_create(desktop_get_main(), "cpu usage", 500, 100, 100, 100, 2, 0, 0);
    desktop_refresh(desktop_get_main());

    while (1) {
        last_idle = idle;
        last_total = total;

        idle = c_process_get_run_time(1);
        total = c_timer_get_ms();

        cpu = 100 - (idle - last_idle) * 100 / (total - last_total);

        for (int i = 0; i < history_size - 1; i++) {
            history[i] = history[i + 1];
        }
        history[history_size - 1] = cpu;

        // reset pixel buffer
        window_fill(window, 0x000000);

        for (int i = 0; i < history_size; i++) {
            for (int j = 0; j < history[i]; j++) {
                window_set_pixel(window, i, 100 - j, 0x00FFFF);
            }
        }

        window_refresh(window);
        ms_sleep(200);
    }
}
