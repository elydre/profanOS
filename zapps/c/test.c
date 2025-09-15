#include <profan/syscall.h>
#include <profan.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define _pscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

void hda_play(uint32_t sample_rate, void* pcm_data, uint32_t pcm_octets, uint16_t lvi);
int hda_is_supported_sample_rate(uint32_t sample_rate);
void hda_set_volume(uint32_t volume);
uint32_t hda_get_pos(void);
void hda_check_headphone_connection_change(void);

#define hda_play(b, c, d, e) ((void) _pscall(7, 0, b, c, d, e))
#define hda_is_supported_sample_rate(b) ((int) _pscall(7, 1, b))
#define hda_set_volume(b) ((void) _pscall(7, 2, b))
#define hda_get_pos() ((uint32_t) _pscall(7, 3, 0))
#define hda_check_headphone_connection_change() ((void) _pscall(7, 4, 0))

typedef struct {
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t length;
    uint32_t flags;
} __attribute__((packed)) hda_play_buffer_t;  

int16_t samples[] = {};

int main(void) {
    void *physical = syscall_mem_alloc(sizeof(samples), 1, 0x1000);
    memcpy(physical, samples, sizeof(samples));

    printf("is playable: %d\n", !hda_is_supported_sample_rate(44100));

    hda_set_volume(30);

    int requested_segments = (sizeof(samples) / (64 * 1024)) + 1;

    hda_play_buffer_t *buf = syscall_mem_alloc(sizeof(hda_play_buffer_t) * 256, 1, 0x1000);

    int av = 0;
    int pos;
    int vol = 30;

    while (av < requested_segments) {
        for (int i = 0; i < 255; i++) {
            buf[i].addr_low = ((uint32_t)physical) + ((i + av) * 64 * 1024);
            buf[i].addr_high = 0;
            buf[i].length = 64 * 1024;
            buf[i].flags = 0;
        }
        asm("wbinvd");

        hda_play(44100, buf, 255 * 44100 * 2, 254);
        printf("refill %d\n", av);

        av += 255;
    
        while ((pos = hda_get_pos()) < ((av - 1) * 64 * 1024)) {
            int sc = syscall_sc_get();
            if (sc == KB_LEFT) {
                vol -= 5;
                if (vol < 0) vol = 0;
                hda_set_volume(vol);
                printf("vol: %d\n", vol);
            }

            if (sc == KB_RIGHT) {
                vol += 5;
                if (vol > 100) vol = 100;
                hda_set_volume(vol);
                printf("vol: %d\n", vol);
            }
            
            if (sc == KB_TOP) {
                printf("check hp\n");
                hda_check_headphone_connection_change();
            }

            serial_debug("pos: %d (block %d)\n", pos, pos / (64 * 1024));
            usleep(1000);
        }
    }

    return 0;
}
