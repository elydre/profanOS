#include <stdlib.h>
#include <stdio.h>
#include <type.h>

#include <syscall.h>
#include <filesys.h>
#include <i_time.h>
#include <i_vgui.h>


#define FILENAME "/user/test.piv"

vgui_t *vgui;

void display_image(uint8_t *data, uint16_t width, uint16_t height) {
    for (uint32_t i = 0; i < width * height; i++) {
        vgui_set_pixel(vgui, i % width, i / width, (data[i * 3] << 16) | (data[i * 3 + 1] << 8) | data[i * 3 + 2]);
    }
    vgui_render(vgui, 0);
}

void play_video(uint8_t *data, uint16_t width, uint16_t height, uint16_t nbframes) {
    for (uint16_t i = 0; i < nbframes; i++) {
        display_image(data + i * width * height * 3, width, height);
        ms_sleep(75);
    }
}

int main(int argc, char **argv) {
    sid_t file = fu_path_to_sid(ROOT_SID, FILENAME);

    if (IS_NULL_SID(file) || !fu_is_file(file)) {
        printf("%s: file not found\n", FILENAME);
        return 1;
    }

    int size = fu_get_file_size(file);
    uint8_t *ptr = malloc(size);
    fu_read_file(file, ptr, size);

    uint8_t type = ptr[0];
    uint16_t width = ptr[1] | (ptr[2] << 8);
    uint16_t height = ptr[3] | (ptr[4] << 8);
    uint16_t nbframes = ptr[5] | (ptr[6] << 8);

    uint8_t *data = ptr + 7;

    vgui_t vgui_obj = vgui_setup(width, height);
    vgui = &vgui_obj;


    if (type == 0xFF) {
        display_image(data, width, height);
    } else if (type == 0xFE) {
        while (1) {
            play_video(data, width, height, nbframes);
        }
    } else {
        printf("Unknown image type: %d\n", type);
    }

    vgui_exit(vgui);

    free(ptr);
    return 0;
}
