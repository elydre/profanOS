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

#define c_dblue     0x01
#define c_dgreen    0x02
#define c_dcyan     0x03
#define c_dred      0x04
#define c_dmagenta  0x05
#define c_dyellow   0x06
#define c_dgrey     0x08


#define kcprint(message, color) kcnprint(message, -1, color)
#define kprint(message) kcprint(message, c_white)

#define FONT_8X16 0

// font.c
uint8_t *font_get(int font_id);

// tefv.c
int  tef_init(void);

// gnrtx
int gt_get_max_cols(void);
int gt_get_max_rows(void);

void clear_screen(void);

void kcnprint(char *message, int len, char color);

void kprint_char_at(int x, int y, char c, char color);

void set_cursor_offset(int offset);
int  get_cursor_offset(void);
int  get_offset(int col, int row);
int  get_offset_row(int offset);
int  get_offset_col(int offset);

void cursor_blink(int on);

#endif
