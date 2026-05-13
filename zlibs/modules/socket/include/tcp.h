/*****************************************************************************\
|   === tcp.h : 2026 ===                                                      |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef TCP_H
#define TCP_H

#include <modules/socket.h>

#define SOCKET_TCP (AF_INET | (SOCK_STREAM << 8) | (0 << 16))

#define TCP_STATE_CLOSED   0
#define TCP_STATE_LISTEN   1
#define TCP_STATE_SYN_SENT 2
#define TCP_STATE_OPEN     3

typedef struct {
    uint8_t state;

    uint16_t local_port;
    uint16_t remote_port;

    uint32_t local_ip;
    uint32_t remote_ip;

    uint8_t tosend[0xffff];
    uint16_t tosend_len;

    uint8_t recv[0xffff];
    uint16_t recv_len;

    uint32_t current_seq;
    uint32_t to_ack;
} tcp_t;

typedef struct {
    uint32_t ip_src;
    uint32_t ip_dest;
    uint16_t port_src;
    uint16_t port_dest;
    uint32_t seq;
    uint32_t ack;
    uint8_t data_offset;
    uint8_t flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_ptr;
    void *option;
    void *data;
    int data_len;
} tcp_packet_t;

int socket_tcp_init(socket_t *sock);
int socket_tcp_bind(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen);
int socket_tcp_connect(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen);
ssize_t socket_tcp_sendto(socket_t *sock, const void *buf, size_t len, int flags,
            const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t socket_tcp_recvfrom(socket_t *sock, void *buf, size_t len, int flags,
            struct sockaddr *src_addr, socklen_t *addrlen);
int socket_tcp_get_rw(socket_t *sock);


void socket_tcp_tick(socket_t *sock);
void socket_on_recv_tcp(uint32_t src_ip, uint32_t dest_ip, uint8_t *data, int data_len);
void socket_on_send_tcp(?);

#endif
