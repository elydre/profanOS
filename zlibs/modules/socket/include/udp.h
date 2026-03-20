/*****************************************************************************\
|   === udp.h : 2026 ===                                                      |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef UDP_H
#define UDP_H

#include <modules/socket.h>
#include <errno.h>

#define SOCKET_UDP (AF_INET | (SOCK_DGRAM << 8) | (0 << 16 ))

typedef struct udp_header_t{
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t len;
    uint16_t checksum;
} __attribute__((packed)) udp_header_t;

typedef struct {
    uint32_t src_ip;
    uint32_t dest_ip;
    uint32_t dest_port;
    uint32_t src_port;
    int len;
    uint8_t *data;
} udp_packet_t;

typedef struct {
    uint32_t local_ip; // any
    uint32_t local_port;
    uint8_t is_bound;

    uint32_t remote_ip;
    uint32_t remote_port;
    uint8_t is_connected;

    udp_packet_t recv[64];
    int recv_len;
    udp_packet_t send[64];
    int send_len;
} udp_t;

int socket_udp_init(socket_t *sock);
int socket_udp_bind(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen);
int socket_udp_connect(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen);
ssize_t socket_udp_sendto(socket_t *sock, const void *buf, size_t len, int flags,
            const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t socket_udp_recvfrom(socket_t *sock, void *buf, size_t len, int flags,
            struct sockaddr *src_addr, socklen_t *addrlen);
int socket_udp_get_rw(socket_t *sock);


void socket_udp_tick(socket_t *sock);
void socket_on_recv_udp(uint32_t src_ip, uint32_t dest_ip, uint8_t *data, int data_len);
void socket_on_send_udp(udp_packet_t *packet);

uint16_t udp_checksum(void *data, int len, uint32_t src_ip, uint32_t dest_ip);
int udp_parse_packet(uint32_t s_ip, uint32_t d_ip, uint8_t *data, int len, udp_packet_t *packet, int verify_checksum);

int udp_is_port_free(uint16_t port);
void udp_lock_port(uint16_t port);
void udp_free_port(uint16_t port);
uint16_t udp_get_free_port();

#endif
