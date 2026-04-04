/*****************************************************************************\
|   === get_rw.c : 2026 ===                                                   |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "udp.h"

int socket_udp_get_rw(socket_t *sock) {
    int res = 0;
    udp_t *data = sock->data;
    if (data->recv_len)
        res |= FM_READ;
    if (data->send_len < 64)
        res |= FM_WRITE;
    return res;
}
