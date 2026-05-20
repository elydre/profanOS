/*****************************************************************************\
|   === update_sock.c : 2026 ===                                              |
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

void tcp_on_packet_recv(tcp_t *sock, tcp_packet_t *packet) {
    switch (sock->state) {
        case TCP_STATE_SYN_SENT: // we sent SYN, but we haven't received SYN+ACK yet
            if ((packet->flags & (TCP_FLAG_SYN | TCP_FLAG_ACK)) == (TCP_FLAG_SYN | TCP_FLAG_ACK)) {
                if (packet->ack != sock->first_seq + 1) {
                    // !TODO send RST
                    return;
                }
                sock->state = TCP_STATE_OPEN;
                sock->first_ack = packet->seq;

                sock->current_seq = sock->first_seq + 1;

                tcp_send_ack(sock);
                sock->last_send = 0;
                sock->retries = 0;
                sock->do_wait_ack = 0;
            }
            break;
        case TCP_STATE_OPEN:
            if (packet->flags & 0x01) { // FIN
                // !TODO do this
            }
            else if (packet->data_len > 0) {
                if (packet->seq - sock->first_ack < sock->current_ack - sock->first_ack) {
                    // TODO check if data overlap after current_ack, if so, we need to accept the new data and reack it
                    // this is a retransmission, ignore it reack it
                    tcp_send_ack(sock);
                }
                else if (packet->seq - sock->first_ack > sock->current_ack - sock->first_ack) {
                    // this is a packet we haven't received yet, ignore it for now
                }
                else {
                    // check if data is longer than our buffer-curesnt size (comunisum)
                    if (!(packet->data_len > sizeof(sock->recv) - sock->recv_len)) {
                        memcpy(sock->recv + sock->recv_len, packet->data, packet->data_len);
                        sock->recv_len += packet->data_len;
                        sock->current_ack += packet->data_len;
                        tcp_send_ack(sock);
                    }
                }

                if (packet->flags & TCP_FLAG_ACK) {
                    size_t data_len = TCP_MIN(sock->tosend_len, TCP_MAX_SEND_ONCE);
                    if (packet->ack - sock->first_seq <= sock->current_seq - sock->first_seq) {
                        // ignore a past ack
                    }
                    else if (packet->ack - sock->first_seq > sock->current_seq - sock->first_seq + data_len) {
                        // this is too far in the future
                    }
                    else {
                        // present ack
                        int to_remove = (packet->ack - sock->first_seq) - (sock->current_seq - sock->first_seq);
                        mem_move(sock->tosend, sock->tosend + to_remove, sock->tosend_len - to_remove);
                        sock->tosend_len -= to_remove;
                    }
                }
            }

            break;
        default:
            break;
    }
}
