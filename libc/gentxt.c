#include <gui/gentxt.h>
#include <gui/vesa.h>

#define FONT_WIDTH 8
#define FONT_HEIGHT 16

void tef_print_char(char c, int *x, int *y, uint32_t color);
void txt_print_at(char *message, int col, int row, char color);
void tef_print(char *message, int x, int y, uint32_t color);

int tef_get_cursor_offset();
int txt_get_cursor_offset();

void tef_set_cursor_offset(int offset);
void txt_set_cursor_offset(int offset);

void txt_backspace();

void tef_clear();
void txt_clear();


uint32_t gt_convert_color(char c) {
    // vga 16 color palette
    switch (c) {
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
        default: return 0xFFFFFF;
    }
}

int gt_get_max_cols() {
    return vesa_does_enable() ? 1024 / FONT_WIDTH : 80;
}

int gt_get_max_rows() {
    return vesa_does_enable() ? 768 / FONT_HEIGHT : 25;
}

void kprint(char *message) {
    if (vesa_does_enable()) {
        tef_print(message, -1, -1, 0xFFFFFF);
    } else {
        txt_print_at(message, -1, -1, c_white);
    }
}

void ckprint(char *message, char color) {
    if (vesa_does_enable()) {
        tef_print(message, -1, -1, gt_convert_color(color));
    } else {
        txt_print_at(message, -1, -1, color);
    }
}

void ckprint_at(char *message, int col, int row, char color) {
    if (vesa_does_enable()) {
        tef_print(message, col, row, gt_convert_color(color));
    } else {
        txt_print_at(message, col, row, color);
    }
}

int get_cursor_offset() {
    if (vesa_does_enable()) {
        return tef_get_cursor_offset();
    } else {
        return txt_get_cursor_offset();
    }
}

void set_cursor_offset(int offset) {
    if (vesa_does_enable()) {
        tef_set_cursor_offset(offset);
    } else {
        txt_set_cursor_offset(offset);
    }
}

void kprint_backspace() {
    if (vesa_does_enable()) {
        int x[1], y[1];
        *x = -1;
        *y = -1;
        tef_print_char(0x08, x, y, 0xFFFFFF);
    } else {
        txt_backspace();
    }
}

void clear_screen() {
    if (vesa_does_enable()) {
        tef_clear();
    } else {
        txt_clear();
    }
}

int get_offset(int col, int row) {
    return 2 * (row * gt_get_max_cols() + col);
}

int get_offset_row(int offset) {
    return offset / (2 * gt_get_max_cols());
}

int get_offset_col(int offset) {
    return (offset - (get_offset_row(offset) * 2 * gt_get_max_cols())) / 2;
}
