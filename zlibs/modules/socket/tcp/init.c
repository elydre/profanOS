/*****************************************************************************\
|   === init.c : 2026 ===                                                     |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "tcp.h"
#include <modules/filesys.h>
#include <fcntl.h>

uint32_t tcp_rand32() {
    int fd = fm_open("/dev/random", O_RDONLY);
    if (fd < 0)
        return 0xAB38FBE1; // random number be like
    uint32_t res = 0;
    fm_read(fd, &res, 4);
    fm_close(fd);
    return res;
}

int socket_tcp_init(socket_t *sock) {
    tcp_t *info = malloc(sizeof(tcp_t));
    if (!info)
        return 1;
    sock->data = info;

    mem_set(info, 0, sizeof(tcp_t));
    info->state = TCP_STATE_CLOSED;
    info->first_seq = tcp_rand32();
    info->current_seq = info->first_seq;

    info->tosend = malloc(TCP_DEFAULT_BUFFER);
    info->recv = malloc(TCP_DEFAULT_BUFFER);
    if (!info->tosend || !info->recv) {
        free(info->tosend);
        free(info->recv);
        free(info);
        sock->data = NULL;
        return 1;
    }
    info->tosend_len = 0;
    info->recv_len = 0;
    info->tosend_max = TCP_DEFAULT_BUFFER;
    info->recv_max = TCP_DEFAULT_BUFFER;
    return 0;
}
