/*****************************************************************************\
|   === select.h : 2025 ===                                                   |
|                                                                             |
|    Implementation of the sys/select.h header file from libC      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#include <profan/minimal.h>
#include <sys/types.h>
#include <sys/time.h>

_BEGIN_C_FILE

#undef FD_SETSIZE
#define FD_SETSIZE  1024

typedef uint32_t __fd_mask;
#define __NFDBITS ((unsigned)(sizeof(__fd_mask) * 8)) // bits per mask
#define __howmany(x, y) (((x) + ((y) - 1)) / (y))

typedef struct fd_set {
    __fd_mask fds_bits[__howmany(FD_SETSIZE, __NFDBITS)];
} fd_set;

void FD_CLR(int fd, fd_set *fdset);
int  FD_ISSET(int fd, const fd_set *fdset);
void FD_SET(int fd, fd_set *fdset);
void FD_ZERO(fd_set *fdset);

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

_END_C_FILE

#endif
