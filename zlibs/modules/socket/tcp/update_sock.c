#include <minilib.h>

#include "tcp.h"

int tcp_send_ack(tcp_t *sock) {
    // !TODO implement this
    return 0;
}

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
                    return;
                }
                else if (packet->seq - sock->first_ack > sock->current_ack - sock->first_ack) {
                    // this is a packet we haven't received yet, ignore it for now
                    return;
                }
                else {
                    // check if data is longer than our buffer-curesnt size (comunisum)
                    if (packet->data_len > sizeof(sock->recv) - sock->recv_len) {
                        // IDK
                        return;
                    }
                    memcpy(sock->recv + sock->recv_len, packet->data, packet->data_len);
                    sock->recv_len += packet->data_len;
                    sock->current_ack += packet->data_len;
                    tcp_send_ack(sock);
                }
            }

            break;
        default:
            break;
    }
}