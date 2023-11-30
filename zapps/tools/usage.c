#include <syscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define HISTOTY_SIZE 80

void buffer_print(uint32_t *pixel_buffer, int x, int y, char *msg) {
    unsigned char *glyph;
    for (int i = 0; msg[i] != '\0'; i++) {
        glyph = c_font_get(0) + msg[i] * 16;
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 8; k++) {
                if (!(glyph[j] & (1 << k))) continue;
                pixel_buffer[i * 8 + x + 8 - k + HISTOTY_SIZE * (y + j)] = 0xffffff;
            }
        }
    }
}

void add_mem_info(uint32_t *pixel_buffer) {
    char tmp[10];

    int alloc_count = c_mem_get_info(4, 0) - c_mem_get_info(5, 0);
    int mem_used = c_mem_get_info(6, 0) / 1024;

    itoa(mem_used, tmp, 10);
    strcat(tmp, "kB");
    buffer_print(pixel_buffer, HISTOTY_SIZE - (strlen(tmp) * 8 + 3), 0, tmp);

    itoa(alloc_count, tmp, 10);
    buffer_print(pixel_buffer, HISTOTY_SIZE - (strlen(tmp) * 8 + 3), 16, tmp);
}

int main(void) {
    uint32_t *fb = c_vesa_get_fb();
    uint32_t pitch = c_vesa_get_pitch();
    uint32_t width = c_vesa_get_width();

    int x_offset = width - HISTOTY_SIZE;
    if (x_offset < 0 || !c_vesa_does_enable()) {
        printf("[cpu] fail to start: screen too small\n");
        return 1;
    }

    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    int *history = calloc(HISTOTY_SIZE, sizeof(int));

    int cpu, last_idle, last_total;
    int idle = 0;
    int total = 0;

    uint32_t *pixel_buffer = calloc(HISTOTY_SIZE * 100, sizeof(uint32_t));

    while (1) {
        last_idle = idle;
        last_total = total;

        idle = c_process_get_run_time(1);
        total = c_timer_get_ms();

        cpu = 100 - (idle - last_idle) * 100 / (total - last_total);

        for (int i = HISTOTY_SIZE - 1; i > 0; i--) {
            history[i] = history[i - 1];
        }
        history[0] = cpu;

        // reset pixel buffer
        for (int i = 0; i < HISTOTY_SIZE * 100; i++) {
            pixel_buffer[i] = 0x000000;
        }

        for (int i = 0; i < HISTOTY_SIZE; i++) {
            for (int j = 0; j < history[i]; j++) {
                pixel_buffer[i + HISTOTY_SIZE * j] = 0x550099;
            }
        }

        add_mem_info(pixel_buffer);

        for (int i = 0; i < HISTOTY_SIZE; i++) {
            for (int j = 0; j < 100; j++) {
                fb[(x_offset + i) % width + j * pitch] = pixel_buffer[i + HISTOTY_SIZE * j];
            }
        }

        c_process_sleep(c_process_get_pid(), 200);
    }
}
