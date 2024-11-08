/*****************************************************************************\
|   === diskiso.h : 2024 ===                                                  |
|                                                                             |
|    Kernel Disk ISO (grub module) header                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef DISKISO_H
#define DISKISO_H

#include <ktype.h>

int      init_diskiso(void);

uint32_t diskiso_get_size(void);
void    *diskiso_get_start(void);
void     diskiso_free(void);

#endif
