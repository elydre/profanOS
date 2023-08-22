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

typedef struct {
    char content;
    char color;
} screen_char_t;

typedef struct {
    uint32_t cursor_x;
    uint32_t cursor_y;
    uint32_t scroll_offset;

    uint32_t max_lines;
    uint32_t max_cols;

    uint32_t saved_cursor_x;
    uint32_t saved_cursor_y;

    uint8_t cursor_is_hidden;

    screen_char_t *screen_buffer;
    
    font_data_t *font;
} panda_global_t;


panda_global_t *g_panda;

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

void print_char(uint32_t xo, uint32_t yo, char c, char color_code) {
    uint32_t color = compute_color(color_code);

    uint8_t *char_data = g_panda->font->data + (c * g_panda->font->charsize);

    uint32_t x = 0;
    uint32_t y = 0;
    for (uint32_t i = 0; i < g_panda->font->charsize; i++) {
        if (x >= g_panda->font->width) {
            x = 0;
            y++;
        }
        for (int j = 7; j >= 0; j--) {
            c_vesa_set_pixel(xo + x, yo + y, char_data[i] & (1 << j) ? color : 0);
            if (x >= g_panda->font->width) break;
            x++;
        }
    }
}


int compute_ansi_escape(char *str) {
    char *start = str;

    if (str[1] == '[') str += 2;
    else return 1;

    // cursor save and restore
    if (str[0] == 's') {
        g_panda->saved_cursor_x = g_panda->cursor_x;
        g_panda->saved_cursor_y = g_panda->cursor_y;
    } else if (str[0] == 'u') {
        g_panda->cursor_x = g_panda->saved_cursor_x;
        g_panda->cursor_y = g_panda->saved_cursor_y;
    }

    // cursor hide and show
    if (strncmp(str, "?25", 3) == 0) {
        if (str[3] == 'l') {
            g_panda->cursor_is_hidden = 0;
        } else if (str[3] == 'h') {
            g_panda->cursor_is_hidden = 1;
        }
        return 5;
    }

    // number
    char *tmp = str;
    while (*tmp >= '0' && *tmp <= '9') tmp++;

    // cursor up
    if (tmp[0] == 'A') {
        int n = atoi(str);
        g_panda->cursor_y -= n;
    }

    // cursor down
    if (tmp[0] == 'B') {
        int n = atoi(str);
        g_panda->cursor_y += n;
    }

    // cursor forward
    if (tmp[0] == 'C') {
        int n = atoi(str);
        g_panda->cursor_x += n;
    }

    // cursor backward
    if (tmp[0] == 'D') {
        int n = atoi(str);
        g_panda->cursor_x -= n;
    }

    return tmp - start;
}

void panda_scroll() {
    // fill the rest of the line with spaces
    int offset;
    while (g_panda->cursor_x < g_panda->max_cols) {
        offset = (g_panda->cursor_y - g_panda->scroll_offset) * g_panda->max_cols + g_panda->cursor_x;
        if (g_panda->screen_buffer[offset].content == '\0') {
            g_panda->screen_buffer[offset].content = ' ';
        }
        g_panda->cursor_x++;
    }

    g_panda->cursor_x = 0;
    g_panda->cursor_y++;

    if (g_panda->cursor_y - g_panda->scroll_offset < g_panda->max_lines) {
        return;
    }
    g_panda->scroll_offset++;

    // scroll the display and print it
    int new_offset;
    for (uint32_t i = 0; i < g_panda->max_lines - 1; i++) {
        for (uint32_t j = 0; j < g_panda->max_cols; j++) {
            new_offset = i * g_panda->max_cols + j;
            offset = new_offset + g_panda->max_cols;
            if (g_panda->screen_buffer[new_offset].content == g_panda->screen_buffer[offset].content &&
                g_panda->screen_buffer[new_offset].color == g_panda->screen_buffer[offset].color
            ) continue;
            g_panda->screen_buffer[new_offset].content = g_panda->screen_buffer[offset].content;
            g_panda->screen_buffer[new_offset].color = g_panda->screen_buffer[offset].color;
            print_char(j * g_panda->font->width, i * g_panda->font->height, g_panda->screen_buffer[new_offset].content, g_panda->screen_buffer[new_offset].color);
        }
    }

    // clear the last line
    for (uint32_t j = 0; j < g_panda->max_cols; j++) {
        new_offset = (g_panda->max_lines - 1) * g_panda->max_cols + j;
        if (g_panda->screen_buffer[new_offset].content == ' ') continue;
        g_panda->screen_buffer[new_offset].content = ' ';
        g_panda->screen_buffer[new_offset].color = 0xF;
        print_char(j * g_panda->font->width, (g_panda->max_lines - 1) * g_panda->font->height, g_panda->screen_buffer[new_offset].content, g_panda->screen_buffer[new_offset].color);
    }
}

void draw_cursor(int errase) {
    uint32_t offset;
    if (!errase) {
        for (uint32_t i = 0; i < g_panda->font->height; i++) {
            c_vesa_set_pixel(g_panda->cursor_x * g_panda->font->width + 1, (g_panda->cursor_y - g_panda->scroll_offset) * g_panda->font->height + i, 0xFFFFFF);
        }
    } else {
        offset = (g_panda->cursor_y - g_panda->scroll_offset) * g_panda->max_cols + g_panda->cursor_x;
        print_char(g_panda->cursor_x * g_panda->font->width,
                  (g_panda->cursor_y - g_panda->scroll_offset) * g_panda->font->height,
                   g_panda->screen_buffer[offset].content,
                   g_panda->screen_buffer[offset].color
        );
    }
}

void panda_print_string(char *string, int len, char color) {
    uint32_t y;
    for (int i = 0; (len < 0) ? (string[i]) : (i < len); i++) {
        if (!g_panda->cursor_is_hidden)
            draw_cursor(1);
        if (string[i] == '\n')
            panda_scroll();
        else if (string[i] == '\033')
            i += compute_ansi_escape(string + i);
        else {
            y = g_panda->cursor_y - g_panda->scroll_offset;
            g_panda->screen_buffer[y * g_panda->max_cols + g_panda->cursor_x].content = string[i];
            g_panda->screen_buffer[y * g_panda->max_cols + g_panda->cursor_x].color = color;
            print_char(g_panda->cursor_x * g_panda->font->width,
                y * g_panda->font->height,
                string[i],
                color
            );
            g_panda->cursor_x++;
        }
        if (g_panda->cursor_x >= g_panda->max_cols)
            panda_scroll();
        if (g_panda->cursor_y - g_panda->scroll_offset >= g_panda->max_lines)
            panda_scroll();
        if (!g_panda->cursor_is_hidden)
            draw_cursor(0);
    }
}

#define offset_to_cursor_y(offset, max_cols) ((offset) / (2 * (max_cols)))

void panda_set_start(int kernel_cursor) {
    uint32_t kmax_cols = c_vesa_get_width() / 8;

    g_panda->cursor_x = 0;
    g_panda->cursor_y = ((offset_to_cursor_y(kernel_cursor, kmax_cols) + 1) * 16) / g_panda->font->height;
    g_panda->scroll_offset = 0;
}

void panda_get_cursor(uint32_t *x, uint32_t *y) {
    *x = g_panda->cursor_x;
    *y = g_panda->cursor_y;
}

void init_panda() {
    g_panda = malloc(sizeof(panda_global_t));
    g_panda->font = load_psf_font("/zada/fonts/lat38-bold18.psf");

    if (g_panda->font == NULL) {
        printf("\n Failed to load font\n");
    }

    g_panda->cursor_x = 0;
    g_panda->cursor_y = 0;

    g_panda->saved_cursor_x = 0;
    g_panda->saved_cursor_y = 0;

    g_panda->scroll_offset = 0;

    g_panda->cursor_is_hidden = 0;

    g_panda->max_lines = c_vesa_get_height() / g_panda->font->height;
    g_panda->max_cols = c_vesa_get_width() / g_panda->font->width;

    g_panda->screen_buffer = calloc(g_panda->max_lines * g_panda->max_cols, sizeof(screen_char_t));
}
