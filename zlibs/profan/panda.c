#include <syscall.h>
#include <filesys.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t charcount;
    uint32_t charsize;

    uint8_t *data;
} font_data_t;

font_data_t *g_font;

void init_panda();

int main(void) {
    init_panda();
    return 0;
}

font_data_t *load_psf_font(char *file) {
    sid_t sid = fu_path_to_sid(ROOT_SID, file);

    if (IS_NULL_SID(sid)) {
        printf("Failed to open font %s\n", file);
        return NULL;
    }

    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t charcount;
    uint32_t charsize;
    uint32_t height;
    uint32_t width;

    fu_file_read(sid, &magic, 0, 4);
    fu_file_read(sid, &version, 4, 4);
    fu_file_read(sid, &headersize, 8, 4);
    fu_file_read(sid, &charcount, 16, 4);
    fu_file_read(sid, &charsize, 20, 4);
    fu_file_read(sid, &height, 24, 4);
    fu_file_read(sid, &width, 28, 4);

    if (magic != 0x864ab572) {
        printf("Invalid magic number\n");
        return NULL;
    }

    if (version != 0) {
        printf("psf version not supported\n");
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

void print_char(uint32_t xo, uint32_t yo, char c, uint32_t color) {
    uint8_t *char_data = g_font->data + (c * g_font->charsize);

    uint32_t x = 0;
    uint32_t y = 0;
    for (uint32_t i = 0; i < g_font->charsize; i++) {
        if (x >= g_font->width) {
            x = 0;
            y++;
        }
        for (int j = 7; j >= 0; j--) {
            c_vesa_set_pixel(xo + x, yo + y, char_data[i] & (1 << j) ? color : 0);
            x++;
        }
    }
}

uint32_t compute_color(char color) {
    uint32_t r = 0;
    uint32_t g = 0;
    uint32_t b = 0;

    if (color & 0x1) {
        r = 0xFF;
    }
    if (color & 0x2) {
        g = 0xFF;
    }
    if (color & 0x4) {
        b = 0xFF;
    }

    return (r << 16) | (g << 8) | b;
}

void panda_print_string(char *string, int len, char color) {
    static uint32_t x = 0;
    static uint32_t y = 0;

    for (int i = 0; (len < 0) ? (string[i]) : (i < len); i++) {
        if (string[i] == '\n') {
            x = 0;
            y += g_font->height;
        } else {
            print_char(x, y, string[i], compute_color(color));
            x += g_font->width;
        }
    }
}


void init_panda() {
    g_font = load_psf_font("/zada/fonts/lat15-bold24.psf");
    if (g_font == NULL) {
        printf("Failed to load font\n");
    }
    printf("Init of panda terminal emulator !\n");
}
