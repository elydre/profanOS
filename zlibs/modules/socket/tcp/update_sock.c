/*****************************************************************************\
|   === update_sock.c : 2026 ===                                              |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <minilib.h>

#include "tcp.h"

void tcp_on_packet_recv(tcp_t *sock, tcp_packet_t *packet) {
    switch (sock->state) {
        case TCP_STATE_SYN_SENT: // we sent SYN, but we haven't received SYN+ACK yet
            if ((packet->flags & (TCP_FLAG_SYN | TCP_FLAG_ACK)) == (TCP_FLAG_SYN | TCP_FLAG_ACK)) {
                if (packet->ack != sock->first_seq + 1) {
                    tcp_send_reset(sock);
                    return;
                }
                sock->state = TCP_STATE_OPEN;
                sock->first_ack = packet->seq;

                sock->current_seq = sock->first_seq + 1;
                sock->current_ack = packet->seq + 1;

                tcp_send_ack(sock);
                sock->retries = 0;
                TCP_CLEAR_INFO(sock, TCP_WAIT_ACK_MASK);

            }
            else if (packet->flags & TCP_FLAG_RST) {
                sock->state = TCP_STATE_CLOSED;
                return ;
            }
            break;
        case TCP_STATE_OPEN:
            if (packet->data_len > 0) {
                if (packet->seq - sock->first_ack < sock->current_ack - sock->first_ack) {
                    // this is a retransmission, ignore it reack it
                    tcp_send_ack(sock);
                }
                else if (packet->seq - sock->first_ack > sock->current_ack - sock->first_ack) {
                    // this is a packet we haven't received yet, ignore it for now
                }
                else {
                    // check if data is longer than our buffer-curesnt size (comunisum)
                    if (!((size_t)packet->data_len > sock->recv_max - sock->recv_len) || sock->recv == NULL) {
                        if (sock->recv != NULL) {
                            mem_copy(sock->recv + sock->recv_len, packet->data, packet->data_len);
                            sock->recv_len += packet->data_len;
                        }
                        sock->current_ack += packet->data_len;
                        tcp_send_ack(sock);
                    }
                }
            }

           if (packet->flags & TCP_FLAG_ACK) {
               size_t data_len = TCP_MIN(sock->tosend_len, TCP_MAX_SEND_ONCE);
               if (packet->ack - sock->first_seq <= sock->current_seq - sock->first_seq) {
                   // ignore a past ack
               }
               else if (packet->ack - sock->first_seq > sock->current_seq - sock->first_seq + data_len +
                            (TCP_GET_INFO(sock, TCP_SEND_FIN_MASK) ? 1 : 0)) {
                   // this is too far in the future
               }
               else {
                    // present ack
                    int to_remove = (packet->ack - sock->first_seq) - (sock->current_seq - sock->first_seq);
                    int do_ack_fin = 0;
                    if ((size_t)to_remove > sock->tosend_len && TCP_GET_INFO(sock, TCP_SEND_FIN_MASK)) {
                        to_remove--;
                        do_ack_fin = 1;
                    }
                    mem_move(sock->tosend, sock->tosend + to_remove, sock->tosend_len - to_remove);
                    sock->tosend_len -= to_remove;
                    sock->retries = 0;
                    TCP_CLEAR_INFO(sock, TCP_WAIT_ACK_MASK);
                    sock->current_seq += to_remove + do_ack_fin;
                    if (do_ack_fin)
                        TCP_SET_INFO(sock, TCP_FIN_ACKED_MASK);
               }
           }
           if (packet->flags & TCP_FLAG_FIN) {
                TCP_SET_INFO(sock, TCP_RECV_FIN_MASK);
                // sock->ack = packet->seq + 1;
                sock->current_ack += 1;
                tcp_send_ack(sock);
           }


            break;
        default:
            break;
    }
}
