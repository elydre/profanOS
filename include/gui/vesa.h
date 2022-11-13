#ifndef VESA_H
#define VESA_H

#include <type.h>

void init_vesa();
void vesa_set_pixel(int x, int y, uint32_t c);
void vesa_clear_screen(uint32_t c);

#endif
