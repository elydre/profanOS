#ifndef VGA_H
#define VGA_H

void vga_320_mode();
void vga_640_mode();
void vga_text_mode();

void vga_put_pixel(unsigned x, unsigned y, unsigned c);
void vga_print(int x, int y, char msg[], int big, unsigned color);
void vga_draw_line(int x1, int y1, int x2, int y2, unsigned color);
void vga_pixel_clear();

int vga_get_width();
int vga_get_height();

#endif
