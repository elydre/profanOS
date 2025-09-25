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
#include <profan.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define HISTOTY_SIZE 80
#define USAGE_HEIGHT 40

void *kfont;

void buffer_print(uint32_t *pixel_buffer, int x, int y, char *msg) {
    unsigned char *glyph;
    for (int i = 0; msg[i] != '\0'; i++) {
        glyph = kfont + msg[i] * 16;
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 8; k++) {
                if (!(glyph[j] & (1 << k)))
                    continue;
                pixel_buffer[i * 8 + x + 8 - k + HISTOTY_SIZE * (y + j)] = 0xBBBBBB;
            }
        }
    }
}

void add_mem_info(uint32_t *pixel_buffer) {
    char tmp[15];

    int mem_used = syscall_mem_info(6, 0) / 1024;

    profan_itoa(mem_used, tmp, 10);
    int len = strlen(tmp);

    if (len < 4) {
        strcpy(tmp + len, " k");
    } else {
        memmove(tmp + len - 2, tmp + len - 3, 2);
        tmp[len - 3] = '.';
        strcat(tmp, " M");
    }

    buffer_print(pixel_buffer, HISTOTY_SIZE - (strlen(tmp) * 8 + 3), 0, tmp);
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

    uint32_t *history = calloc(HISTOTY_SIZE, sizeof(int));

    int last_idle, last_total, last_kernel;
    int idle = 0;
    int total = 0;
    int kernel = 0;

    uint32_t *pixel_buffer = calloc(HISTOTY_SIZE * USAGE_HEIGHT, sizeof(uint32_t));

    kfont = syscall_font_get();

    while (1) {
        last_idle = idle;
        last_total = total;
        last_kernel = kernel;

        idle   = syscall_process_run_time(1);
        kernel = syscall_process_run_time(0);
        total = syscall_ms_get();

        int s = (total - last_total ?: 1);
        last_total = USAGE_HEIGHT - ((idle - last_idle) * USAGE_HEIGHT / s);
        last_kernel = (kernel - last_kernel) * USAGE_HEIGHT / s;

        for (int i = HISTOTY_SIZE - 1; i > 0; i--)
            history[i] = history[i - 1];

        history[0] = (last_total << 16) | last_kernel;

        for (int i = 0; i < HISTOTY_SIZE; i++) {
            last_total = (history[i] >> 16) & 0xff;
            last_kernel = history[i] & 0xff;
            for (int j = 0; j < USAGE_HEIGHT; j++) {
                if (j < last_kernel)
                    pixel_buffer[i + HISTOTY_SIZE * j] = 0xa26ea8;
                else if (j < last_total)
                    pixel_buffer[i + HISTOTY_SIZE * j] = 0x711b7d;
                else
                    pixel_buffer[i + HISTOTY_SIZE * j] = 0;
            }
        }

        add_mem_info(pixel_buffer);

        for (int j = 0; j <= USAGE_HEIGHT; j++)
            fb[x_offset - 1 + j * pitch] = 0x444444;

        for (int i = 0; i < HISTOTY_SIZE; i++) {
            for (int j = 0; j < USAGE_HEIGHT; j++) {
                fb[(x_offset + i) % width + j * pitch] = pixel_buffer[i + HISTOTY_SIZE * j];
            }
            fb[(x_offset + i) % width + USAGE_HEIGHT * pitch] = 0x666666;
        }

        syscall_process_sleep(syscall_process_pid(), 200);
    }
}
