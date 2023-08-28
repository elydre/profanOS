#ifndef VESA_H
#define VESA_H

#include <ktype.h>

void     init_vesa();
int      vesa_does_enable();

void     vesa_set_pixel(int x, int y, uint32_t c);
uint32_t vesa_get_pixel(int x, int y);

void    *vesa_get_framebuffer();

uint32_t vesa_get_pitch();
int      vesa_get_width();
int      vesa_get_height();

#endif
