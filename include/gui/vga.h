#ifndef VGA_H
#define VGA_H

void vga_init();
void vga_put_pixel(int x, int y, unsigned char color);
int vga_get_width();
int vga_get_height();
void vga_clear_screen();

void vga_switch_text_mode(int hi_res);

#endif