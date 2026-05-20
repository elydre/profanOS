/*****************************************************************************\
|   === tick.c : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <minilib.h>

#include "tcp.h"

#define TCP_TIMEOUT 500 // 500ms
#define TCP_MAX_RETRIES 5

void socket_tcp_tick(tcp_t *sock) {
    uint32_t now = timer_get_ms();

    switch (sock->state) {
        case TCP_STATE_SYN_SENT:
            if (sock->do_wait_ack && sock->last_send + TCP_TIMEOUT < now) {
                tcp_send_syn(sock);
                sock->last_send = now;
                sock->retries++;
            }
            break;
        case TCP_STATE_OPEN:
            if (sock->do_wait_ack && sock->last_send + TCP_TIMEOUT < now) {
                tcp_send_data(sock);
                sock->last_send = now;
                sock->retries++;
            }
            else if (sock->tosend_len > 0 && !sock->do_wait_ack) {
                tcp_send_data(sock);
                sock->last_send = now;
                sock->retries = 0;
                sock->do_wait_ack = 1;

            }
            // !TODO implement timeout for OPEN§0
            break;
        default:
            break;
    }

    if (sock->retries > TCP_MAX_RETRIES) {
        // !TODO RESET
    }
}
