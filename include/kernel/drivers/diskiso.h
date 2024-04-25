/****** This file is part of profanOS **************************\
|   == diskiso.h ==                                  .pi0iq.    |
|                                                   d"  . `'b   |
|                                                   q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef DISKISO_H
#define DISKISO_H

#include <ktype.h>

int      init_diskiso(void);

uint32_t diskiso_get_size(void);
uint32_t diskiso_get_start(void);
void     diskiso_free(void);

#endif
