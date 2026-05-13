void udpate_sock_tcp(tcp_t *sock, tcp_packet_t *packet) {
    switch (sock->state) {
        case TCP_STATE_LISTEN:
            if (packet->flags & 0x02) { // SYN
                sock->remote_ip = packet->ip_src;
                sock->remote_port = packet->port_src;
                sock->seq = 0; // !TODO: randomize
                sock->ack = packet->seq + 1;
                sock->state = TCP_STATE_SYN_SENT;
            }
            break;
        case TCP_STATE_SYN_SENT:
            if ((packet->flags & 0x12) == 0x12) { // SYN+ACK
                sock->ack = packet->seq + 1;
                sock->state = TCP_STATE_OPEN;
            }
            break;
        case TCP_STATE_OPEN:
            if (packet->flags & 0x01) { // FIN
                sock->state = TCP_STATE_CLOSED;
            } else if (packet->data_len > 0) {
            // pleqse do the rest
            }
            break;
    }
}