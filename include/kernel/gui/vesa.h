/*****************************************************************************\
|   === vesa.h : 2024 ===                                                     |
|                                                                             |
|    Kernel grub VESA driver header                                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

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
