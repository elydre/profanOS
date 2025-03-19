/*****************************************************************************\
|   === select.c : 2025 ===                                                   |
|                                                                             |
|    Implementation of sys/select functions from libC              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <sys/select.h>
#include <profan.h>

void FD_SET(int fd, fd_set *p) {
    p->fds_bits[fd / __NFDBITS] |= (1U << (fd % __NFDBITS));
}

void FD_CLR(int fd, fd_set *p) {
    p->fds_bits[fd / __NFDBITS] &= ~(1U << (fd % __NFDBITS));
}

int FD_ISSET(int fd, const fd_set *p) {
    return p->fds_bits[fd / __NFDBITS] & (1U << (fd % __NFDBITS));
}

void FD_ZERO(fd_set *p) {
    size_t _n = __howmany(FD_SETSIZE, __NFDBITS);
    while (_n > 0)
        p->fds_bits[--_n] = 0;
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    return (PROFAN_FNI, -1);
}
