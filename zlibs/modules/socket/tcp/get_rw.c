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

#include "tcp.h"

int socket_tcp_get_rw(socket_t *sock) {
    int res = 0;
    tcp_t *data = sock->data;

    if (data->state == TCP_STATE_OPEN) {
        if (data->recv_len > 0 && !TCP_GET_INFO(data, TCP_RECV_FIN_MASK))
            res |= FM_READ;
        if (data->tosend_len < data->tosend_max && !TCP_GET_INFO(data, TCP_SEND_FIN_MASK))
            res |= FM_WRITE;
    }

    if (data->state == TCP_STATE_SYN_SENT)
        res |= FM_WRITE;

    return res;
}
