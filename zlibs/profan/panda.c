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
    switch (color) {
        case 0x0: return 0x000000;
        case 0x1: return 0x0000AA;
        case 0x2: return 0x00AA00;
        case 0x3: return 0x00AAAA;
        case 0x4: return 0xAA0000;
        case 0x5: return 0xAA00AA;
        case 0x6: return 0xAA5500;
        case 0x7: return 0xAAAAAA;
        case 0x8: return 0x555555;
        case 0x9: return 0x5555FF;
        case 0xA: return 0x55FF55;
        case 0xB: return 0x55FFFF;
        case 0xC: return 0xFF5555;
        case 0xD: return 0xFF55FF;
        case 0xE: return 0xFFFF55;
        default:  return 0xFFFFFF;
    }
}

void panda_print_string(char *string, int len, char color) {
    static uint32_t x = 0;
    static uint32_t y = 0;

    for (int i = 0; (len < 0) ? (string[i]) : (i < len); i++) {
        if (string[i] == '\n') {
            x = 0;
            y += g_font->height;
            if (y >= c_vesa_get_height() - g_font->height) {
                y = 0;
            }
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
