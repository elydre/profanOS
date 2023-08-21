#include <syscall.h>
#include <filesys.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t charcount;
    uint32_t charsize;

    uint8_t *data;
} font_data_t;

void print_string(font_data_t *font, uint32_t y, char *s);

font_data_t *load_psf_font(char *file) {
    sid_t sid = fu_path_to_sid(ROOT_SID, file);

    if (IS_NULL_SID(sid)) {
        printf("Failed to open font %s\n", file);
        return NULL;
    }

    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t charcount;
    uint32_t charsize;
    uint32_t height;
    uint32_t width;

    fu_file_read(sid, &magic, 0, 4);
    fu_file_read(sid, &version, 4, 4);
    fu_file_read(sid, &headersize, 8, 4);
    fu_file_read(sid, &flags, 12, 4);
    fu_file_read(sid, &charcount, 16, 4);
    fu_file_read(sid, &charsize, 20, 4);
    fu_file_read(sid, &height, 24, 4);
    fu_file_read(sid, &width, 28, 4);

    if (magic != 0x864ab572) {
        printf("Invalid magic number\n");
        return NULL;
    }

    uint8_t *font = malloc(charcount * charsize);
    fu_file_read(sid, font, headersize, charcount * charsize);

    font_data_t *psf = malloc(sizeof(font_data_t));
    psf->width = width;
    psf->height = height;
    psf->charcount = charcount;
    psf->charsize = charsize;
    psf->data = font;

    return psf;
}

void free_font(font_data_t *pff) {
    free(pff->data);
    free(pff);
}

void fontdemo(char *file) {
    static uint32_t y = 0;

    font_data_t *font = load_psf_font(file);
    if (font == NULL) {
        return;
    }

    print_string(font, y, file);
    y += font->height;
    free_font(font);
}

int main(void) {
    for (int i = 0; i < 400; i++) {
        for (int j = 0; j < 200; j++) {
            c_vesa_set_pixel(i, j, 0);
        }
    }

    fontdemo("/zada/fonts/font.psf");
    fontdemo("/zada/fonts/zap.psf");

    return 0;
}

void print_char(font_data_t *font, uint32_t xo, uint32_t yo, char c) {
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

void print_string(font_data_t *font, uint32_t y, char *s) {
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
