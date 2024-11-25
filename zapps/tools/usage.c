/*****************************************************************************\
|   === usage.c : 2024 ===                                                    |
|                                                                             |
|    Graphical CPU usage monitor with memory informations          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define HISTOTY_SIZE 80

void *kfont;

void buffer_print(uint32_t *pixel_buffer, int x, int y, char *msg) {
    unsigned char *glyph;
    for (int i = 0; msg[i] != '\0'; i++) {
        glyph = kfont + msg[i] * 16;
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

    int alloc_count = syscall_mem_info(4, 0) - syscall_mem_info(5, 0);
    int mem_used = syscall_mem_info(6, 0) / 1024;

    itoa(mem_used, tmp, 10);
    strcat(tmp, "kB");
    buffer_print(pixel_buffer, HISTOTY_SIZE - (strlen(tmp) * 8 + 3), 0, tmp);

    itoa(alloc_count, tmp, 10);
    buffer_print(pixel_buffer, HISTOTY_SIZE - (strlen(tmp) * 8 + 3), 16, tmp);
}

int main(void) {
    uint32_t *fb = syscall_vesa_fb();
    uint32_t pitch = syscall_vesa_pitch();
    uint32_t width = syscall_vesa_width();

    int x_offset = width - HISTOTY_SIZE;
    if (x_offset < 0 || !syscall_vesa_state()) {
        printf("[cpu] fail to start: screen too small\n");
        return 1;
    }

    int *history = calloc(HISTOTY_SIZE, sizeof(int));

    int cpu, last_idle, last_total;
    int idle = 0;
    int total = 0;

    uint32_t *pixel_buffer = calloc(HISTOTY_SIZE * 100, sizeof(uint32_t));

    kfont = syscall_font_get();

    while (1) {
        last_idle = idle;
        last_total = total;

        idle = syscall_process_run_time(1);
        total = syscall_timer_get_ms();

        cpu = total - last_total;
        cpu = 100 - (idle - last_idle) * 100 / (cpu ? cpu : 1);

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

        syscall_process_sleep(syscall_process_pid(), 200);
    }
}
