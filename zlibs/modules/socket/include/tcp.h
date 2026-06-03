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
#include <minilib.h>

#define SOCKET_TCP (AF_INET | (SOCK_STREAM << 8) | (0 << 16))

#define TCP_DEFAULT_BUFFER 0xFFFF

#define TCP_STATE_CLOSED   0
#define TCP_STATE_LISTEN   1
#define TCP_STATE_SYN_SENT 2
#define TCP_STATE_OPEN     3

#define TCP_FLAG_FIN (1 << 0)
#define TCP_FLAG_SYN (1 << 1)
#define TCP_FLAG_RST (1 << 2)
#define TCP_FLAG_PSH (1 << 3)
#define TCP_FLAG_ACK (1 << 4)
#define TCP_FLAG_URG (1 << 5)

#define TCP_MAX_SEND_ONCE (1024)
#define TCP_MIN(A, B) (A < B ? A : B)

#define TCP_BIND_MASK (1 << 0)
#define TCP_CONNECT_MASK (1 << 1)
#define TCP_WAIT_ACK_MASK (1 << 2)
#define TCP_RECV_FIN_MASK (1 << 3)
#define TCP_SEND_FIN_MASK (1 << 4)
#define TCP_FIN_ACKED_MASK (1 << 5)

#define TCP_GET_INFO(X, MASK) ((X)->mask_info & MASK)
#define TCP_SET_INFO(X, MASK) ((X)->mask_info |= MASK)
#define TCP_CLEAR_INFO(X, MASK) ((X)->mask_info &= ~MASK)

typedef struct {
    uint8_t *tosend;
    uint8_t *recv;

    size_t recv_len;
    size_t recv_max;

    size_t tosend_len;
    size_t tosend_max;

    uint32_t local_ip;
    uint32_t remote_ip;

    uint32_t first_seq;
    uint32_t current_seq;
    uint32_t first_ack; // first seq sent by the other side
    uint32_t current_ack;
    uint32_t last_send;

    uint16_t local_port;
    uint16_t remote_port;

    uint8_t state;
    uint8_t retries;
    uint8_t mask_info;
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
int socket_tcp_shutdown(socket_t *sock, int how);

void tcp_on_packet_recv(tcp_t *sock, tcp_packet_t *packet);

void socket_on_recv_tcp(uint32_t src_ip, uint32_t dest_ip, uint8_t *data, int data_len);
void socket_tcp_tick(socket_t *sock_ptr);

void tcp_send_syn(tcp_t *sock);
void tcp_send_data(tcp_t *sock);
void tcp_send_ack(tcp_t *sock);
void tcp_send_reset(tcp_t *sock);

int tcp_is_port_free(uint16_t port);
void tcp_lock_port(uint16_t port);
void tcp_free_port(uint16_t port);
uint16_t tcp_get_free_port();
uint32_t tcp_rand32();

#endif
