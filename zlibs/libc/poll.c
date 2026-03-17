/*****************************************************************************\
|   === poll.c : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/filesys.h>
#include <profan/syscall.h>

#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    int res = 0;

    uint32_t t_start = syscall_timer_get_ms();
    uint32_t time_end = t_start + timeout;

    while ((timeout < 1 || syscall_timer_get_ms() < time_end) && res == 0) {
        for (int i = 0; i < nfds; i++) {
            if (fds[i].fd == -1)
                continue;
            int rw = fm_get_fd_rw(fds[i].fd);
            if (rw < 0)
                fds[i].revents |= POLLNVAL;
            else {
                if ((rw & FM_READ) && (fds[i].events & POLLIN))
                    fds[i].revents |= POLLIN;
                if ((rw & FM_WRITE) && (fds[i].events & POLLOUT))
                    fds[i].revents |= POLLOUT;
            }
            if (fds[i].revents)
                res++;
        }
        if (timeout == 0)
            break;
    }

    return res;
}
