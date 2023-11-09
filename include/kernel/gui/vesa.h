#ifndef VESA_H
#define VESA_H

#include <ktype.h>

void     init_vesa(void);
int      vesa_does_enable(void);

void     vesa_set_pixel(int x, int y, uint32_t c);
uint32_t vesa_get_pixel(int x, int y);

void    *vesa_get_framebuffer(void);

uint32_t vesa_get_pitch(void);
int      vesa_get_width(void);
int      vesa_get_height(void);

#endif
