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
#include <cpu/timer.h>

#include "tcp.h"

#define TCP_TIMEOUT 500 // 500ms
#define TCP_MAX_RETRIES 5

void socket_tcp_tick(tcp_t *sock) {
    uint32_t now = timer_get_ms();

    switch (sock->state) {
        case TCP_STATE_SYN_SENT:
            if (TCP_GET_INFO(sock, TCP_WAIT_ACK_MASK) && sock->last_send + TCP_TIMEOUT < now) {
                tcp_send_syn(sock);
                sock->retries++;
            }
            break;
        case TCP_STATE_OPEN:
            if (TCP_GET_INFO(sock, TCP_WAIT_ACK_MASK) && sock->last_send + TCP_TIMEOUT < now) {
                tcp_send_data(sock);
                sock->retries++;
            }
            else if (sock->tosend_len > 0 && !TCP_GET_INFO(sock, TCP_WAIT_ACK_MASK)) {
                tcp_send_data(sock);
                sock->retries = 0;
				TCP_SET_INFO(sock, TCP_WAIT_ACK_MASK);
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
