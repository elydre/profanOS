/*****************************************************************************\
|   === mouse.h : 2024 ===                                                    |
|                                                                             |
|    Kernel Mouse driver header                                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef MOUSE_H
#define MOUSE_H

#include <cpu/isr.h>
#include <ktype.h>

int  mouse_init(void);
void mouse_handler(registers_t *a_r);

int  mouse_call(int thing, int val);

#endif
