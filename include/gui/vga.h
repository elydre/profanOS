#ifndef VGA_H
#define VGA_H

void vga_320_mode();
void vga_640_mode();
void vga_text_mode();

void vga_set_pixel(unsigned x, unsigned y, unsigned c);

void vga_clear_screen();

int vga_get_width();
int vga_get_height();

int vga_get_mode();
void vga_switch_mode(int mode);

#endif
