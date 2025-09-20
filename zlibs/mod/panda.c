/*****************************************************************************\
|   === panda.c : 2024 ===                                                    |
|                                                                             |
|    Terminal emulator as kernel module with psf font              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <kernel/snowflake.h>
#include <kernel/afft.h>
#include <gui/gnrtx.h>

#include <gui/vesa.h>
#include <minilib.h>

#include <profan/panda.h>
#include <profan.h>

#define DEFAULT_FONT "/zada/fonts/lat38-bold18.psf"
#define SCROLL_LINES 8

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t charcount;
    uint32_t charsize;

    uint8_t *data;
} font_data_t;

typedef struct {
    int cursor_x;
    int cursor_y;
    int scroll_offset;

    int max_lines;
    int max_cols;

    int saved_cursor_x;
    int saved_cursor_y;

    uint8_t cursor_is_hidden;
    uint8_t always_bright;

    uint16_t color;

    uint32_t *fb;
    uint32_t pitch;

    panda_char_t *screen_buffer;

    font_data_t *font;
} panda_global_t;

panda_global_t *g_panda;

#define set_pixel(x, y, color) \
    g_panda->fb[(x) + (y) * g_panda->pitch] = color

static font_data_t *load_psf_font(uint32_t sid) {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t charcount;
    uint32_t charsize;
    uint32_t height;
    uint32_t width;

    fs_cnt_read(sid, &magic, 0, 4);
    fs_cnt_read(sid, &version, 4, 4);
    fs_cnt_read(sid, &headersize, 8, 4);
    fs_cnt_read(sid, &charcount, 16, 4);
    fs_cnt_read(sid, &charsize, 20, 4);
    fs_cnt_read(sid, &height, 24, 4);
    fs_cnt_read(sid, &width, 28, 4);

    if (magic != 0x864ab572 || version != 0)
        return NULL;

    uint8_t *font = malloc(charcount * charsize);
    fs_cnt_read(sid, font, headersize, charcount * charsize);

    font_data_t *psf = malloc(sizeof(font_data_t));
    psf->width = width;
    psf->height = height;
    psf->charcount = charcount;
    psf->charsize = charsize;
    psf->data = font;

    return psf;
}

static void free_font(font_data_t *pff) {
    free(pff->data);
    free(pff);
}

static inline uint32_t compute_color(uint8_t color) {
    uint32_t rgb[] = {
        0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
        0xAA0000, 0xAA00AA, 0xAA8800, 0xBBBBBB,
        0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
        0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
    };

    if (color > 0xF) return 0xFFFFFF;
    return rgb[(int) color];
}

static inline void print_char(uint32_t xo, uint32_t yo, uint8_t c, uint16_t color_code) {
    uint32_t bg_color = compute_color((color_code >> 4) & 0xF);
    uint32_t fg_color = compute_color(color_code & 0xF);

    uint8_t *char_data = g_panda->font->data + (c * g_panda->font->charsize);

    uint32_t x = 0;
    uint32_t y = 0;

    for (uint32_t i = 0; i < g_panda->font->charsize; i++) {
        for (int j = 7; j >= 0; j--) {
            g_panda->fb[(xo + x) + (yo + y) * g_panda->pitch] = char_data[i] & (1 << j) ? fg_color : bg_color;
            if (x == g_panda->font->width - 1) {
                x = 0;
                y++;
                break;
            }
            x++;
        }
    }

    if (color_code & PANDA_UNDERLINE) {
        for (uint32_t i = 0; i < g_panda->font->width; i++) {
            set_pixel(xo + i, yo + g_panda->font->height - 1, fg_color);
        }
    }
}

void panda_print_char(uint32_t x, uint32_t y, uint8_t c, uint16_t color) {
    uint32_t offset = y * g_panda->max_cols + x;

    if (g_panda->screen_buffer[offset].content == c &&
        g_panda->screen_buffer[offset].color   == color
    ) return;

    g_panda->screen_buffer[offset].content = c;
    g_panda->screen_buffer[offset].color   = color;

    print_char(x * g_panda->font->width, y * g_panda->font->height, c, color);
}

void panda_print_raw(panda_char_t *buffer, uint32_t length) {
    if (length > (uint32_t)(g_panda->max_cols * g_panda->max_lines))
        length = g_panda->max_cols * g_panda->max_lines;

    for (uint32_t i = 0; i < length; i++) {
        if (g_panda->screen_buffer[i].content == buffer[i].content &&
            g_panda->screen_buffer[i].color   == buffer[i].color
        ) continue;

        g_panda->screen_buffer[i].content = buffer[i].content;
        g_panda->screen_buffer[i].color   = buffer[i].color;

        print_char(
                (i % g_panda->max_cols) * g_panda->font->width,
                (i / g_panda->max_cols) * g_panda->font->height,
                buffer[i].content, buffer[i].color
        );
    }
}

static void panda_clear_screen(void) {
    if (!g_panda) return;
    for (int i = 0; i < g_panda->max_lines; i++) {
        for (int j = 0; j < g_panda->max_cols; j++) {
            panda_print_char(j, i, ' ', 0x0F);
        }
    }
    g_panda->cursor_x = 0;
    g_panda->cursor_y = 0;
    g_panda->scroll_offset = 0;
}

static uint16_t compute_ansi_color(char ansi_nb, int part, uint16_t old_color) {
    char fg = old_color & 0xF;
    char bg = (old_color >> 4) & 0xF;

    int conv[] = {
        0, 4, 2, 6, 1, 5, 3, 7
    };

    if (ansi_nb < 0 || ansi_nb > 7)
        ansi_nb = 7;
    else
        ansi_nb = conv[(int) ansi_nb];

    if (part == 0) {
        fg = ansi_nb;
    } else if (part == 1) {
        fg = ansi_nb + 8;
    } else if (part == 2) {
        bg = ansi_nb;
    } else {
        bg = ansi_nb + 8;
    }

    return (bg << 4) | fg | (old_color & PANDA_UNDERLINE);
}

static int compute_ansi_escape(const char *str, int main_color) {
    const char *start = str;

    if (str[0] == '\e' && str[1] == '[')
        str += 2;
    else
        return -1;

    int vals[4] = {0};
    int private = 0;
    int vcount = 0;

    if (*str == '?') {
        private = 1;
        str++;
    }

    while (*str >= '0' && *str <= '9' && vcount < 4) {
        while (*str >= '0' && *str <= '9') {
            vals[vcount] = vals[vcount] * 10 + (*str - '0');
            str++;
        }
        vcount++;
        if (*str == ';') {
            str++;
        } else {
            break;
        }
    }

    switch (*str) {
        case 's': // save cursor position
            if (vcount > 0 || private)
                goto UNKNOWN_ESCAPE;
            g_panda->saved_cursor_x = g_panda->cursor_x;
            g_panda->saved_cursor_y = g_panda->cursor_y;
            break;
        case 'u': // restore cursor position
            if (vcount > 0 || private)
                goto UNKNOWN_ESCAPE;
            if (g_panda->saved_cursor_y < g_panda->scroll_offset)
                g_panda->saved_cursor_y = g_panda->scroll_offset;
            else
                g_panda->cursor_y = g_panda->saved_cursor_y;
            g_panda->cursor_x = g_panda->saved_cursor_x;
            break;
        case 'K': // clear line
            if (vcount > 1 || private)
                goto UNKNOWN_ESCAPE;
            if (vcount == 0 || vals[0] == 0)
                for (int i = g_panda->cursor_x; i < g_panda->max_cols; i++)
                    panda_print_char(i, g_panda->cursor_y - g_panda->scroll_offset, ' ', g_panda->color);
            else if (vals[0] == 1) // clear from start to cursor
                for (int i = 0; i < g_panda->cursor_x; i++)
                    panda_print_char(i, g_panda->cursor_y - g_panda->scroll_offset, ' ', g_panda->color);
            else if (vals[0] == 2) // clear whole line
                for (int i = 0; i < g_panda->max_cols; i++)
                    panda_print_char(i, g_panda->cursor_y - g_panda->scroll_offset, ' ', g_panda->color);
            else
                goto UNKNOWN_ESCAPE;
            break;
        case 'm': // set text attributes
            if (vcount == 0 || private)
                goto UNKNOWN_ESCAPE;
            for (int i = 0; i < vcount; i++) {
                if (vals[i] == 0) {
                    g_panda->color = main_color == -1 ? 0xF : main_color;
                    g_panda->color &= ~PANDA_UNDERLINE;
                    g_panda->always_bright = 0;
                } else if (vals[i] == 4) {
                    g_panda->color |= PANDA_UNDERLINE; // underline
                } else if (vals[i] == 24) {
                    g_panda->color &= ~PANDA_UNDERLINE; // end underline
                } else if (vals[i] == 1) {  // bold
                    g_panda->always_bright = 1;
                } else if (vals[i] == 22) { // end bold
                    g_panda->always_bright = 0;
                } else if (vals[i] >= 30 && vals[i] <= 37) {
                    g_panda->color = compute_ansi_color(vals[i] - 30, 0, g_panda->color);
                } else if (vals[i] >= 40 && vals[i] <= 47) {
                    g_panda->color = compute_ansi_color(vals[i] - 40, 2, g_panda->color);
                } else if (vals[i] >= 90 && vals[i] <= 97) {
                    g_panda->color = compute_ansi_color(vals[i] - 90, 1, g_panda->color);
                } else if (vals[i] >= 100 && vals[i] <= 107) {
                    g_panda->color = compute_ansi_color(vals[i] - 100, 3, g_panda->color);
                } else {
                    goto UNKNOWN_ESCAPE;
                }
            }
            break;
        case 'l': // show cursor
            if (private && vcount == 1 && vals[0] == 25)
                g_panda->cursor_is_hidden = 0;
            else
                goto UNKNOWN_ESCAPE;
            break;
        case 'h': // hide cursor
            if (private && vcount == 1 && vals[0] == 25)
                g_panda->cursor_is_hidden = 1;
            else
                goto UNKNOWN_ESCAPE;
            break;
        case 'J': // clear screen
            if (vcount > 1 || private)
                goto UNKNOWN_ESCAPE;
            if (vals[0] == -1 || vals[0] == 0) { // clear from cursor to end of screen
                for (int i = g_panda->cursor_y - g_panda->scroll_offset; i < g_panda->max_lines; i++) {
                    for (int j = 0; j < g_panda->max_cols; j++) {
                        panda_print_char(j, i, ' ', g_panda->color);
                    }
                }
            } else if (vals[0] == 1) { // clear from start of screen to cursor
                for (int i = 0; i < g_panda->cursor_y - g_panda->scroll_offset; i++) {
                    for (int j = 0; j < g_panda->max_cols; j++) {
                        panda_print_char(j, i, ' ', g_panda->color);
                    }
                }
            } else if (vals[0] == 2) { // clear whole screen
                panda_clear_screen();
            } else {
                goto UNKNOWN_ESCAPE;
            }
            break;
        case 'f':
        case 'H': // move cursor to position
            if (private)
                goto UNKNOWN_ESCAPE;
            if (*str == 'H' && vcount == 0) {
                // set cursor to top left
                g_panda->cursor_x = 0;
                g_panda->cursor_y = 0;
                g_panda->scroll_offset = 0;
            } else if (vcount == 2) {
                // set cursor to position
                if (vals[0] < 0)
                    vals[0] = 0;
                if (vals[1] < 0)
                    vals[1] = 0;
                if (vals[0] >= g_panda->max_lines + g_panda->scroll_offset)
                    vals[0] = g_panda->max_lines - 1 + g_panda->scroll_offset;
                if (vals[1] >= g_panda->max_cols)
                    vals[1] = g_panda->max_cols - 1;
                g_panda->cursor_y = vals[0] + g_panda->scroll_offset;
                g_panda->cursor_x = vals[1];
            } else {
                goto UNKNOWN_ESCAPE;
            }
            break;
        case 'A': // cursor up
            if (private || vcount > 1)
                goto UNKNOWN_ESCAPE;
            if (vcount == 0 || vals[0] < 1)
                g_panda->cursor_y--;
            else
                g_panda->cursor_y -= vals[0];
            if (g_panda->cursor_y < g_panda->scroll_offset)
                g_panda->cursor_y = g_panda->scroll_offset;
            break;
        case 'B': // cursor down
            if (private || vcount > 1)
                goto UNKNOWN_ESCAPE;
            if (vcount == 0 || vals[0] < 1)
                g_panda->cursor_y++;
            else
                g_panda->cursor_y += vals[0];
            if (g_panda->cursor_y >= g_panda->max_lines + g_panda->scroll_offset)
                g_panda->cursor_y = g_panda->max_lines - 1 + g_panda->scroll_offset;
            break;
        case 'C': // cursor forward
            if (private || vcount > 1)
                goto UNKNOWN_ESCAPE;
            if (vcount == 0 || vals[0] < 1)
                g_panda->cursor_x++;
            else
                g_panda->cursor_x += vals[0];
            if (g_panda->cursor_x >= g_panda->max_cols) {
                g_panda->cursor_x -= g_panda->max_cols;
                g_panda->cursor_y++;
            }
            if (g_panda->cursor_y >= g_panda->max_lines + g_panda->scroll_offset)
                g_panda->cursor_y = g_panda->max_lines - 1 + g_panda->scroll_offset;
            break;
        case 'D': // cursor backward
            if (private || vcount > 1)
                goto UNKNOWN_ESCAPE;
            if (vcount == 0 || vals[0] < 1)
                g_panda->cursor_x--;
            else
                g_panda->cursor_x -= vals[0];
            if (g_panda->cursor_x < 0) {
                g_panda->cursor_x += g_panda->max_cols;
                g_panda->cursor_y--;
            }
            if (g_panda->cursor_y < g_panda->scroll_offset)
                g_panda->cursor_y = g_panda->scroll_offset;
            break;
        default:
            // unknown escape sequence
            goto UNKNOWN_ESCAPE;
    }

    return str - start;

    UNKNOWN_ESCAPE:

    return -(str - start); // return negative value to indicate error
}

static void panda_scroll(uint32_t line_count) {
    int offset;

    g_panda->cursor_x = 0;
    g_panda->cursor_y++;

    if (g_panda->cursor_y - g_panda->scroll_offset < g_panda->max_lines) {
        // fill new line with spaces
        for (int j = 0; j < g_panda->max_cols; j++) {
            panda_print_char(j, g_panda->cursor_y - g_panda->scroll_offset, ' ', 0xF);
        }
        return;
    }

    g_panda->scroll_offset += line_count;

    // scroll the display and print it
    for (uint32_t i = 0; i < g_panda->max_lines - line_count; i++) {
        for (int j = 0; j < g_panda->max_cols; j++) {
            offset = i * g_panda->max_cols + j + g_panda->max_cols * line_count;
            panda_print_char(j, i, g_panda->screen_buffer[offset].content, g_panda->screen_buffer[offset].color);
        }
    }

    // clear the last line
    for (uint32_t i = 0; i < line_count; i++) {
        for (int j = 0; j < g_panda->max_cols; j++) {
            panda_print_char(j, g_panda->max_lines - 1 - i, ' ', 0xF);
        }
    }
}

static void draw_cursor(int errase) {
    uint32_t offset;
     if (!errase) {
        for (uint32_t i = 0; i < g_panda->font->height; i++) {
            set_pixel(g_panda->cursor_x * g_panda->font->width + 1,
                    (g_panda->cursor_y - g_panda->scroll_offset) * g_panda->font->height + i, 0xFFFFFF);
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

uint16_t panda_print_string(const char *string, int len, int string_color, uint16_t default_color) {
    if (!g_panda) return 0;
    int tmp, old_color;

    static char ansi_buffer[16] = "";

    if (string_color != -1) {
        old_color = g_panda->color;
        g_panda->color = string_color;
    }

    if (ansi_buffer[0] != '\0') {
        // if there is an ansi escape sequence in progress, we need to finish it
        unsigned i, old_len;
        for (i = 0; i < sizeof(ansi_buffer) - 1 && ansi_buffer[i]; i++);
        old_len = i;
        for (int j = 0; (len < 0 ? string[j] : j < len) && i < sizeof(ansi_buffer) - 1; j++)
            ansi_buffer[i++] = string[j];
        ansi_buffer[i] = '\0';

        int ansi_len = compute_ansi_escape(ansi_buffer, default_color);

        if (ansi_len >= 0) {
            string += ansi_len - old_len + 1;
            len -= ansi_len - old_len + 1;
        } else if (-ansi_len == len) {
            // incomplete escape sequence
            goto END;
        }

        ansi_buffer[0] = '\0';
    }

    for (int i = 0; len < 0 ? string[i] : i < len; i++) {
        if (!g_panda->cursor_is_hidden)
            draw_cursor(1);
        if (string[i] == '\n')
            panda_scroll(SCROLL_LINES);
        else if (string[i] == '\r')
            g_panda->cursor_x = 0;
        else if (string[i] == '\a')
            // bell (do nothing)
            ;
        else if (string[i] == '\b') {
            if (g_panda->cursor_x > 0)
                g_panda->cursor_x--;
        } else if (string[i] == '\t') {
            tmp = g_panda->cursor_x + 4 - (g_panda->cursor_x % 4);
            for (; g_panda->cursor_x < tmp; g_panda->cursor_x++)
                panda_print_char(g_panda->cursor_x, g_panda->cursor_y - g_panda->scroll_offset, ' ', g_panda->color);
        } else if (string[i] == '\e') {
            // copy to the ansi buffer
            unsigned j;
            for (j = 0; j < sizeof(ansi_buffer) - 1 && (len < 0 ? string[i + j] : (int) j + i < len); j++)
                ansi_buffer[j] = string[i + j];
            ansi_buffer[j] = '\0';

            int ansi_len = compute_ansi_escape(ansi_buffer, default_color);
            if (ansi_len >= 0) {
                i += ansi_len;
            } else if (-ansi_len == len - i) {
                goto END; // we have an incomplete escape sequence, so we stop here
            }
            ansi_buffer[0] = '\0';
        } else {
            panda_print_char(g_panda->cursor_x, g_panda->cursor_y - g_panda->scroll_offset, string[i],
                    g_panda->always_bright ? (g_panda->color | 0x08) : g_panda->color);
            g_panda->cursor_x++;
        }

        if (g_panda->cursor_x >= g_panda->max_cols)
            panda_scroll(SCROLL_LINES);
        if (g_panda->cursor_y - g_panda->scroll_offset >= g_panda->max_lines)
            panda_scroll(SCROLL_LINES);
        if (!g_panda->cursor_is_hidden)
            draw_cursor(0);
    }

    END:

    if (string_color == -1)
        return g_panda->color;

    string_color = g_panda->color;
    g_panda->color = old_color;
    return string_color;
}

#define offset_to_cursor_y(offset, max_cols) ((offset) / (2 * (max_cols)))

void panda_sync_start(void) {
    if (!g_panda)
        return;

    g_panda->cursor_x = 0;
    g_panda->cursor_y = ((offset_to_cursor_y(cursor_get_offset(),
            vesa_get_info(0) / 8) + 1) * 16) / g_panda->font->height;

    g_panda->scroll_offset = 0;
}

void panda_get_size(uint32_t *x, uint32_t *y) {
    if (!g_panda) {
        *x = 0;
        *y = 0;
    } else {
        *x = g_panda->max_cols;
        *y = g_panda->max_lines;
    }
}

void panda_get_cursor(uint32_t *x, uint32_t *y) {
    if (!g_panda) {
        *x = 0;
        *y = 0;
    } else {
        *x = g_panda->cursor_x;
        *y = g_panda->cursor_y;
    }
}

void panda_draw_cursor(uint32_t x, uint32_t y) {
    if (!g_panda) return;
    draw_cursor(1);
    g_panda->cursor_x = x;
    g_panda->cursor_y = y;
    draw_cursor(0);
}

int panda_change_font(uint32_t sid) {
    if (!g_panda || IS_SID_NULL(sid))
        return 1;

    font_data_t *font = load_psf_font(sid);

    if (font == NULL)
        return 1;

    panda_clear_screen();
    free_font(g_panda->font);
    g_panda->font = font;
    g_panda->max_lines = vesa_get_info(1) / g_panda->font->height;
    g_panda->max_cols = vesa_get_info(0) / g_panda->font->width;
    g_panda->screen_buffer = realloc(g_panda->screen_buffer,
            g_panda->max_lines * g_panda->max_cols * sizeof(panda_char_t));

    return 0;
}

void *panda_screen_backup(void) {
    if (!g_panda)
        return NULL;

    font_data_t *font = mem_alloc(sizeof(font_data_t), SNOW_SIMPLE, 0);
    mem_copy(font, g_panda->font, sizeof(font_data_t));
    font->data = mem_alloc(font->charcount * font->charsize, SNOW_SIMPLE, 0);
    mem_copy(font->data, g_panda->font->data, font->charcount * font->charsize);

    panda_char_t *screen = mem_alloc(g_panda->max_lines * g_panda->max_cols * sizeof(panda_char_t), SNOW_SIMPLE, 0);
    mem_copy(screen, g_panda->screen_buffer, g_panda->max_lines * g_panda->max_cols * sizeof(panda_char_t));

    panda_global_t *panda = mem_alloc(sizeof(panda_global_t), SNOW_SIMPLE, 0);
    mem_copy(panda, g_panda, sizeof(panda_global_t));
    panda->font = font;
    panda->screen_buffer = screen;

    return panda;
}

void panda_screen_restore(void *data) {
    panda_global_t *source = (panda_global_t *) data;

    if (!g_panda || !data)
        return;

    draw_cursor(1);

    // restore font
    g_panda->font->data = realloc( g_panda->font->data, source->font->charcount * source->font->charsize);
    mem_copy(g_panda->font->data, source->font->data, source->font->charcount * source->font->charsize);
    g_panda->font->charcount = source->font->charcount;
    g_panda->font->charsize = source->font->charsize;
    g_panda->font->height = source->font->height;
    g_panda->font->width = source->font->width;

    // restore screen buffer
    g_panda->screen_buffer = realloc(g_panda->screen_buffer,
            source->max_lines * source->max_cols * sizeof(panda_char_t));
    mem_copy(g_panda->screen_buffer, source->screen_buffer,
            source->max_lines * source->max_cols * sizeof(panda_char_t));

    // restore other fields
    g_panda->cursor_is_hidden = source->cursor_is_hidden;
    g_panda->scroll_offset = source->scroll_offset;
    g_panda->max_lines = source->max_lines;
    g_panda->max_cols = source->max_cols;
    g_panda->cursor_x = source->cursor_x;
    g_panda->cursor_y = source->cursor_y;
    g_panda->color = source->color;

    for (int i = 0; i < g_panda->max_lines; i++) {
        for (int j = 0; j < g_panda->max_cols; j++) {
            print_char(j * g_panda->font->width, i * g_panda->font->height,
                    g_panda->screen_buffer[i * g_panda->max_cols + j].content,
                    g_panda->screen_buffer[i * g_panda->max_cols + j].color
            );
        }
    }

    if (!g_panda->cursor_is_hidden)
        draw_cursor(0);
}

void panda_screen_kfree(void *data) {
    panda_global_t *panda = (panda_global_t *) data;
    if (!panda)
        return;

    free_font(panda->font);
    free(panda->screen_buffer);
    free(panda);
}

static int dev_panda_r(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(offset);

    static char *buffer_addr = NULL;
    static uint32_t already_read = 0;

    if (buffer_addr == NULL) {
        buffer_addr = profan_input_keyboard(NULL, "/dev/panda");
        already_read = 0;
    }

    uint32_t to_read = size;
    uint32_t buffer_size = str_len(buffer_addr);

    if (already_read + to_read > buffer_size) {
        to_read = buffer_size - already_read;
    }

    mem_copy(buffer, buffer_addr + already_read, to_read);
    already_read += to_read;

    if (already_read >= buffer_size) {
        free(buffer_addr);
        buffer_addr = NULL;
    }

    return to_read;
}

static int dev_panda_w(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(offset);

    panda_print_string((char *) buffer, size, -1, 0x0F);

    return size;
}

static int dev_pander_w(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(offset);

    static uint8_t color = 0x0C;

    color = panda_print_string((char *) buffer, size, color, 0x0C);
    return size;
}

static int setup_afft(const char *name, void *write) {
    int afft_id = afft_register(AFFT_AUTO, dev_panda_r, write, NULL);

    return (afft_id == -1 || kfu_afft_create("/dev", name, afft_id) == SID_NULL);
}

int __init(void) {
    if (!vesa_get_info(4)) {
        kprintf("[panda] VESA is not enabled\n");
        g_panda = NULL;
        return 1;
    }

    g_panda = malloc(sizeof(panda_global_t));
    g_panda->font = load_psf_font(kfu_path_to_sid(SID_ROOT, DEFAULT_FONT));

    if (g_panda->font == NULL) {
        kprintf("[panda] Failed to load font\n");
        free(g_panda);
        g_panda = NULL;
        return 1;
    }

    g_panda->cursor_x = 0;
    g_panda->cursor_y = 0;

    g_panda->saved_cursor_x = 0;
    g_panda->saved_cursor_y = 0;

    g_panda->scroll_offset = 0;

    g_panda->cursor_is_hidden = 1;
    g_panda->always_bright = 0;
    g_panda->color = 0x0F;

    g_panda->max_lines = vesa_get_info(1) / g_panda->font->height;
    g_panda->max_cols = vesa_get_info(0) / g_panda->font->width;

    g_panda->fb = (void *) vesa_get_info(3);
    g_panda->pitch = vesa_get_info(2);

    g_panda->screen_buffer = calloc(g_panda->max_lines * g_panda->max_cols * sizeof(panda_char_t));

    return (setup_afft("panda", dev_panda_w) || setup_afft("pander", dev_pander_w));
}
