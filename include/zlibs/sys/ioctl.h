/*****************************************************************************\
|   === ioctl.h : 2026 ===                                                    |
|                                                                             |
|    Implementation of the sys/ioctl.h header file from libC       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

#include <profan/minimal.h>

_BEGIN_C_FILE

int ioctl(int fd, unsigned long op, ...);

_END_C_FILE

#endif
