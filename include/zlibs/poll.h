/*****************************************************************************\
|   === poll.h : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef POLL_H
#define POLL_H

struct pollfd {
    int   fd;
    short events;
    short revents;
};

#define POLLIN (1 << 0)
#define POLLPRI (1 << 1)
#define POLLOUT (1 << 2)
#define POLLRDHUP (1 << 3)
#define POLLERR (1 << 4)
#define POLLHUP (1 << 5)
#define POLLNVAL (1 << 6)

typedef int nfds_t;

int poll(struct pollfd *fds, nfds_t nfds, int timeout);

#endif
