#include <syscall.h>
#include <stdlib.h>
#include <i_vgui.h>
#include <stdio.h>


int main(int argc, char **argv) {
    int history_size = 100;

    int *history = calloc(history_size, sizeof(int));

    vgui_t vgui_obj = vgui_setup(1024, 101);
    vgui_t *vgui = &vgui_obj;

    int cpu, last_idle, last_total;
    int idle = 0;
    int total = 0;

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

        vgui_draw_rect(vgui, 1024 - 101, 0, 101, 101, 0x000000);
        for (int i = 0; i < history_size; i++) {
            vgui_draw_line(vgui, 1024 - i, 100 - history[i], 1024 - i, 100, 0x00FFFF);
        }

        vgui_render(vgui, 0);
        c_process_sleep(c_process_get_pid(), 200);
    }
}
