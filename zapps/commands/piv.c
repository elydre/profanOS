#include <syscall.h>
#include <stdlib.h>
#include <i_time.h>
#include <stdio.h>
#include <type.h>

#define FILENAME "/user/test.piv"

void display_image(uint8_t *data, uint16_t width, uint16_t height) {
    for (uint32_t i = 0; i < width * height; i++) {
        c_vesa_set_pixel(i % width, i / width, (data[i * 3] << 16) | (data[i * 3 + 1] << 8) | data[i * 3 + 2]);
    }
}

void play_video(uint8_t *data, uint16_t width, uint16_t height, uint16_t nbframes) {
    for (uint16_t i = 0; i < nbframes; i++) {
        display_image(data + i * width * height * 3, width, height);
        ms_sleep(75);
    }
}

int main(int argc, char *argv[]) {
    int size = c_fs_get_file_size(FILENAME);
    uint8_t *ptr = (uint8_t *) malloc(size);

    c_fs_read_file(FILENAME, ptr);

    uint8_t type = ptr[0];
    uint16_t width = ptr[1] | (ptr[2] << 8);
    uint16_t height = ptr[3] | (ptr[4] << 8);
    uint16_t nbframes = ptr[5] | (ptr[6] << 8);

    uint8_t *data = ptr + 7;

    if (type == 0xFF) {
        display_image(data, width, height);
    } else if (type == 0xFE) {
        while (1) {
            play_video(data, width, height, nbframes);
        }
    } else {
        printf("Unknown image type: %d\n", type);
    }

    free(ptr);
    return 0;
}
