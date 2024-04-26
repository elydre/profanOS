/*****************************************************************************\
|   === multiboot.h : 2024 ===                                                |
|                                                                             |
|    Kernel Multiboot header                                       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <ktype.h>

void     mboot_save(void *mboot_ptr);
uint32_t mboot_get(int index);

#endif
