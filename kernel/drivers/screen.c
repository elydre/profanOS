#include <gui/gnrtx.h>
#include <cpu/ports.h>
#include <minilib.h>
#include <ktype.h>


/**********************************************************
 * Public Kernel API functions                            *
***********************************************************/

#define MAX_ROWS 25
#define MAX_COLS 80

#define VIDEO_ADDRESS 0xb8000

// Screen i/o ports
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

void txt_set_cursor_offset(int offset);
int  txt_get_cursor_offset(void);

void txt_print_char(char c, char attr) {
    int offset = txt_get_cursor_offset();
    int row = get_offset_row(offset);

    uint8_t *vidmem = (uint8_t*) VIDEO_ADDRESS;

    if (c == '\n') {
        row = get_offset_row(offset);
        offset = get_offset(0, row+1);
    } else if (c == '\r') {
        offset = get_offset(0, row);
    } else if (c == 0x08) {
        // str_backspace
        vidmem[offset] = ' ';
        vidmem[offset+1] = attr;
    } else {
        vidmem[offset] = c;
        vidmem[offset+1] = attr;
        offset += 2;
    }

    // Check if the offset is over screen size and scroll
    if (offset >= MAX_ROWS * MAX_COLS * 2) {
        int i;
        for (i = 1; i < MAX_ROWS; i++)
            mem_copy((uint8_t*)(get_offset(0, i-1) + VIDEO_ADDRESS),
                    (uint8_t*)(get_offset(0, i) + VIDEO_ADDRESS),
                    MAX_COLS * 2);

        // Blank last line
        char *last_line = (char*) (get_offset(0, MAX_ROWS-1) + (uint8_t*) VIDEO_ADDRESS);
        for (i = 0; i < MAX_COLS * 2; i++) last_line[i] = 0;

        offset -= 2 * MAX_COLS;
    }

    txt_set_cursor_offset(offset);
}

void txt_set_char(int x, int y, char c, char color) {
    int offset = get_offset(x, y);
    if (offset >= MAX_ROWS * MAX_COLS * 2) return;
    uint8_t *vidmem = (uint8_t*) VIDEO_ADDRESS;
    vidmem[offset] = c;
    vidmem[offset+1] = color;
}

void txt_clear(void) {
    int screen_size = MAX_COLS * MAX_ROWS;
    int i;
    uint8_t *screen = (uint8_t*) VIDEO_ADDRESS;

    for (i = 0; i < screen_size; i++) {
        screen[i*2] = ' ';
        screen[i*2+1] = 0x00;
    }
    txt_set_cursor_offset(get_offset(0, 0));
}

int txt_get_cursor_offset(void) {
    /* Use the VGA ports to get the current cursor position
     * 1. Ask for high byte of the cursor offset (data 14)
     * 2. Ask for low byte (data 15) */

    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8; // High byte: << 8
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset * 2; // position * size of character cell
}

void txt_set_cursor_offset(int offset) {
    // Similar to get_cursor_offset, but instead of reading we write data
    offset /= 2;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (uint8_t)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (uint8_t)(offset & 0xff));
}

void txt_cursor_blink(int on) {
    port_byte_out(REG_SCREEN_CTRL, 0x0A);
    port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xC0) | (on ? 0x0F : 0x0C));
}
