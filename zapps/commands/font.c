#include <syscall.h>
#include <filesys.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
pff file format:
[0-3] width
[4-7] height
[8-11] charcount
[12...] char data
*/

#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t charcount;
    uint8_t *data;
} pff_font_t;

void print_string(pff_font_t *font, uint32_t y, char *s);

pff_font_t *pff_load_font(char *file) {
    sid_t sid = fu_path_to_sid(ROOT_SID, file);

    if (IS_NULL_SID(sid)) {
        return NULL;
    }

    uint32_t width;
    uint32_t height;
    uint32_t charcount;

    fu_file_read(sid, &width, 0, 4);
    fu_file_read(sid, &height, 4, 4);
    fu_file_read(sid, &charcount, 8, 4);

    uint32_t char_size = (ALIGN_UP(width, 8) * height) / 8;
    uint8_t *font = malloc(char_size * charcount);
    fu_file_read(sid, font, 12, char_size * charcount);

    pff_font_t *pff = malloc(sizeof(pff_font_t));
    pff->width = width;
    pff->height = height;
    pff->charcount = charcount;
    pff->data = font;

    return pff;
}

void pff_free_font(pff_font_t *pff) {
    free(pff->data);
    free(pff);
}

void fontdemo(char *file) {
    static uint32_t y = 0;

    pff_font_t *font = pff_load_font(file);
    print_string(font, y, file);
    y += font->height;
    pff_free_font(font);
}

int main(void) {
    for (int i = 0; i < 400; i++) {
        for (int j = 0; j < 200; j++) {
            c_vesa_set_pixel(i, j, 0);
        }
    }
    fontdemo("/zada/fonts/ter16x32.pff");
    fontdemo("/zada/fonts/sun12x22.pff");
    fontdemo("/zada/fonts/font10x18.pff");
    fontdemo("/zada/fonts/font8x16.pff");
    fontdemo("/zada/fonts/sun8x16.pff");

    return 0;
}

// c_vesa_set_pixel(x, y, color);

void print_char(pff_font_t *font, uint32_t xo, uint32_t yo, char c) {
    uint32_t w = ALIGN_UP(font->width, 8);
    uint32_t h = font->height;

    uint8_t *char_data = font->data + (c * (w * h) / 8);
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = w / 8 - 1; x < w / 8; x--) {
            uint8_t byte = char_data[y * w / 8 + x];
            for (uint32_t i = 0; i < 8; i++) {
                c_vesa_set_pixel(xo + ((x + 1) * 8) - i, yo + y, byte & (1 << i) ? 0xFFFFFF : 0);
            }
        }
    }
}

void print_string(pff_font_t *font, uint32_t y, char *s) {
    uint32_t x = 0;
    while (*s) {
        if (*s == '\n') {
            x = 0;
            y += font->height;
        } else {
            print_char(font, x, y, *s);
            x += font->width;
        }
        s++;
    }
}
