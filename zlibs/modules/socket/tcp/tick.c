/*****************************************************************************\
|   === tick.c : 2026 ===                                                     |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <minilib.h>
#include <cpu/timer.h>

#include "tcp.h"
#include "utils.h"

#define TCP_TIMEOUT 500 // 500ms
#define TCP_MAX_RETRIES 5

void socket_tcp_tick(socket_t *sock_ptr) {
    uint32_t now = timer_get_ms();
    tcp_t *sock = sock_ptr->data;
    if (sock->state == TCP_STATE_CLOSED && sock_ptr->ref_count == 0) {
        tcp_free_port(htons(sock->local_port));
        free(sock->recv);
        free(sock->tosend);
        free(sock);
        sock_ptr->data = NULL;
        sock_ptr->do_remove = 1;
        sock_ptr->type = 0;
        return;
    }

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
            else if (((TCP_GET_INFO(sock, TCP_SEND_FIN_MASK) && !TCP_GET_INFO(sock, TCP_FIN_ACKED_MASK)) || sock->tosend_len > 0) &&
                            !TCP_GET_INFO(sock, TCP_WAIT_ACK_MASK)) {
                tcp_send_data(sock);
                sock->retries = 0;
                TCP_SET_INFO(sock, TCP_WAIT_ACK_MASK);
            }
            // check if both are FIN and ACKed, if so, we can close the connection
            if (TCP_GET_INFO(sock, TCP_SEND_FIN_MASK) && TCP_GET_INFO(sock, TCP_RECV_FIN_MASK) && sock->recv_len == 0) {
                sock->state = TCP_STATE_CLOSED;
                free(sock->recv);
                sock->recv = NULL;
                sock->recv_len = 0;
                sock->recv_max = 0;
            }
            break;
        default:
            break;
    }

    if (sock->retries > TCP_MAX_RETRIES) {
        // !TODO RESET
    }
}
