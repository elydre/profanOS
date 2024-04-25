/****** This file is part of profanOS **************************\
|   == vesa.h ==                                     .pi0iq.    |
|                                                   d"  . `'b   |
|   Kernel grub VESA driver header                  q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef VESA_H
#define VESA_H

#include <ktype.h>

void     init_vesa(void);
void     vesa_set_pixel(int x, int y, uint32_t c);

#define vesa_get_width()    vesa_get_info(0)
#define vesa_get_height()   vesa_get_info(1)
#define vesa_get_pitch()    vesa_get_info(2)
#define vesa_get_fb() (void *) vesa_get_info(3)
#define vesa_does_enable()  vesa_get_info(4)

uint32_t vesa_get_info(int id);

#endif
