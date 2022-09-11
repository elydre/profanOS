#ifndef SCREEN_H
#define SCREEN_H

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

/* Screen i/o ports */
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5


typedef enum ScreenColor ScreenColor;
enum ScreenColor {
    // light colors
    c_blue = 0x09,
    c_green = 0x0a,
    c_cyan = 0x0b,
    c_red = 0x0c,
    c_magenta = 0x0d,
    c_yellow = 0x0e,
    c_grey = 0x07,
    c_white = 0x0f,
    
    // dark colors
    c_dblue = 0x01,
    c_dgreen = 0x02,
    c_dcyan = 0x03,
    c_dred = 0x04,
    c_dmagenta = 0x05,
    c_dyellow = 0x06,
    c_dgrey = 0x08,
};

/* Public kernel API */
void clear_screen();

void kprint(char *message);
void ckprint(char *message, char color);
void ckprint_at(char *message, int col, int row, char color);
int print_char(char c, int col, int row, char attr);
void kprint_backspace();


void set_cursor_offset(int offset);
int get_cursor_offset();
int get_offset(int col, int row);
int get_offset_row(int offset);
int get_offset_col(int offset);

#endif
