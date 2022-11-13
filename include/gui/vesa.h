#ifndef VESA_H
#define VESA_H

#include <type.h>

void init_vesa();
int vesa_does_enable();
void vesa_set_pixel(int x, int y, uint32_t c);

#endif
