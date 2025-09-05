/*****************************************************************************\
|   === gnrtx.c : 2024 ===                                                    |
|                                                                             |
|    Kernel ansii escape multi-terminal support                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <minilib.h>

#define FONT_HEIGHT 16
#define FONT_WIDTH  8

void tef_set_char(int x, int y, char c, char color);
void tef_print_char(char c, char color);
void tef_cursor_set_offset(int offset);
int  tef_cursor_get_offset(void);
void tef_cursor_blink(int on);
void tef_clear(void);

void txt_set_char(int x, int y, char c, char color);
void txt_print_char(char c, char color);
void txt_cursor_set_offset(int offset);
void txt_cursor_blink(int on);
int  txt_cursor_get_offset(void);
void txt_clear(void);

int gt_get_max_cols(void) {
    return vesa_does_enable() ? vesa_get_width() / FONT_WIDTH : 80;
}

int gt_get_max_rows(void) {
    return vesa_does_enable() ? vesa_get_height() / FONT_HEIGHT : 25;
}

void kprint_char_at(int x, int y, char c, char color) {
    if (x < 0 || y < 0) {
        if (vesa_does_enable()) {
            tef_print_char(c, color);
        } else {
            txt_print_char(c, color);
        }
    } else {
        if (vesa_does_enable()) {
            tef_set_char(x, y, c, color);
        } else {
            txt_set_char(x, y, c, color);
        }
    }
}

int cursor_get_offset(void) {
    if (vesa_does_enable()) {
        return tef_cursor_get_offset();
    } else {
        return txt_cursor_get_offset();
    }
}

void cursor_set_offset(int offset) {
    if (vesa_does_enable()) {
        tef_cursor_set_offset(offset);
    } else {
        txt_cursor_set_offset(offset);
    }
}

void clear_screen(void) {
    if (vesa_does_enable()) {
        tef_clear();
    } else {
        txt_clear();
    }
}

int cursor_calc_offset(int col, int row) {
    return 2 * (row * gt_get_max_cols() + col);
}

int cursor_calc_row(int offset) {
    return offset / (2 * gt_get_max_cols());
}

int cursor_calc_col(int offset) {
    return (offset - (cursor_calc_row(offset) * 2 * gt_get_max_cols())) / 2;
}

void cursor_blink(int on) {
    if (vesa_does_enable()) {
        tef_cursor_blink(on);
    } else {
        txt_cursor_blink(on);
    }
}

uint32_t saved_cursor_x = 0;
uint32_t saved_cursor_y = 0;

char compute_ansi_color(char ansi_nb, int part, char old_color) {
    char fg = old_color & 0xF;
    char bg = (old_color >> 4) & 0xF;

    switch (ansi_nb) {
        case '0': ansi_nb = 0; break;
        case '1': ansi_nb = 4; break;
        case '2': ansi_nb = 2; break;
        case '3': ansi_nb = 6; break;
        case '4': ansi_nb = 1; break;
        case '5': ansi_nb = 5; break;
        case '6': ansi_nb = 3; break;
        default:  ansi_nb = 7; break;
    }

    if (part == 0) {
        fg = ansi_nb;
    } else if (part == 1) {
        fg = ansi_nb + 8;
    } else if (part == 2) {
        bg = ansi_nb;
    }

    return (bg << 4) | fg;
}

int compute_ansi_escape(const char *str, char *color) {
    const char *start = str;

    if (str[1] == '[') str += 2;
    else return 1;

    // cursor save and restore
    if (str[0] == 's') {
        saved_cursor_x = cursor_calc_col(cursor_get_offset());
        saved_cursor_y = cursor_calc_row(cursor_get_offset());
    } else if (str[0] == 'u') {
        cursor_set_offset(cursor_calc_offset(saved_cursor_x, saved_cursor_y));
    } else if (str[0] == 'K') {
        int offset = cursor_get_offset();
        int row = cursor_calc_row(offset);
        int col = cursor_calc_col(offset);
        for (int i = col; i < gt_get_max_cols() - 1; i++) {
            kprint_char_at(-1, -1, ' ', 0x0);
        }
        cursor_set_offset(cursor_calc_offset(col, row));
    }

    // cursor hide and show
    if (str_ncmp(str, "?25", 3) == 0) {
        if (str[3] == 'l') {
            cursor_blink(1);
        } else if (str[3] == 'h') {
            cursor_blink(0);
        }
        return 5;
    }

    // font color
    if (str[0] == '3' && str[2] == 'm') {
        *color = compute_ansi_color(str[1], 0, *color);
        return 4;
    }

    // highlight font color
    if (str[0] == '9' && str[2] == 'm') {
        *color = compute_ansi_color(str[1], 1, *color);
        return 4;
    }

    // background color
    if (str[0] == '4' && str[2] == 'm') {
        *color = compute_ansi_color(str[1], 2, *color);
        return 4;
    }

    // reset color
    if (str[0] == '0' && str[1] == 'm') {
        *color = 0x0F;
        return 3;
    }

    // clear screen
    if (str[0] == '2' && str[1] == 'J') {
        clear_screen();
        return 3;
    }
    // set top left
    if (str[0] == 'H') {
        cursor_set_offset(0);
        return 2;
    }

    // number
    const char *tmp = str;
    while (*tmp >= '0' && *tmp <= '9') tmp++;

    // cursor up
    if (tmp[0] == 'A') {
        cursor_set_offset(cursor_get_offset() - str2int(str) * 2 * gt_get_max_cols());
    }

    // cursor down
    else if (tmp[0] == 'B') {
        cursor_set_offset(cursor_get_offset() + str2int(str) * 2 * gt_get_max_cols());
    }

    // cursor forward
    else if (tmp[0] == 'C') {
        cursor_set_offset(cursor_get_offset() + str2int(str) * 2);
    }

    // cursor backward
    else if (tmp[0] == 'D') {
        cursor_set_offset(cursor_get_offset() - str2int(str) * 2);
    }

    return tmp - start;
}

int kcnprint(const char *message, int len, char color) {
    int i = 0;

    if (len == -1)
        len = str_len(message);

    while (i < len) {
        if (message[i] == '\e')
            i += compute_ansi_escape(message + i, &color);
        else
            kprint_char_at(-1, -1, message[i], color);
        i++;
    }

    return len;
}
