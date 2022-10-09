#ifndef VGA_H
#define VGA_H

void vga_pixel_mode();
void vga_text_mode();

void vga_put_pixel(int x, int y, unsigned char color);
void vga_pixel_clear();

int vga_get_width();
int vga_get_height();

#endif
