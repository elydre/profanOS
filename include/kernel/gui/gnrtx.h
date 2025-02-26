/*****************************************************************************\
|   === gnrtx.h : 2024 ===                                                    |
|                                                                             |
|    Kernel multi output terminal header                           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef GNRTX_H
#define GNRTX_H

#include <ktype.h>

#define c_blue      0x09
#define c_green     0x0a
#define c_cyan      0x0b
#define c_red       0x0c
#define c_magenta   0x0d
#define c_yellow    0x0e
#define c_grey      0x07
#define c_white     0x0f

#define kcprint(message, color) kcnprint(message, -1, color)
#define kprint(message) kcprint(message, c_white)

// font.c
extern uint8_t font_8x16[4096];
uint8_t *font_get(void);

// tefv.c
int  tef_init(void);

// gnrtx
int gt_get_max_cols(void);
int gt_get_max_rows(void);

void clear_screen(void);

int kcnprint(const char *message, int len, char color);

void kprint_char_at(int x, int y, char c, char color);

void cursor_set_offset(int offset);
int  cursor_get_offset(void);
int  cursor_calc_offset(int col, int row);
int  cursor_calc_row(int offset);
int  corsor_calc_col(int offset);

void cursor_blink(int on);

#endif
