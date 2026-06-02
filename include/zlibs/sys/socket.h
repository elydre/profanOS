/*****************************************************************************\
|   === socket.h : 2026 ===                                                   |
|                                                                             |
|    Implementation of the sys/socket.h header file from libC      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>
#include <unistd.h>

#define AF_UNIX     1
#define AF_LOCAL    1
#define AF_INET     2
#define AF_INET6    10
#define AF_PACKET   17
#define AF_NETLINK  16

#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17


#define MSG_OOB         0x1
#define MSG_PEEK        0x2
#define MSG_DONTROUTE   0x4
#define MSG_EOR         0x8 /* data completes record */
#define MSG_TRUNC       0x10    /* data discarded before delivery */
#define MSG_CTRUNC      0x20    /* control data lost before delivery */
#define MSG_WAITALL     0x40    /* wait for full request or error */
#define MSG_DONTWAIT    0x80    /* this message should be nonblocking */
#define MSG_BCAST       0x100   /* this message rec'd as broadcast */
#define MSG_MCAST       0x200   /* this message rec'd as multicast */
#define MSG_NOSIGNAL    0x400   /* do not send SIGPIPE */
#define MSG_CMSG_CLOEXEC0x800   /* set FD_CLOEXEC on received fds */
#define MSG_WAITFORONE  0x1000  /* nonblocking but wait for one msg */
#define MSG_CMSG_CLOFORK0x2000  /* set FD_CLOFORK on received fds */

typedef uint32_t socklen_t;
typedef uint16_t sa_family_t;
typedef uint32_t in_addr_t;
typedef uint16_t in_port_t;

struct sockaddr {
    sa_family_t sa_family;
    char sa_data[14];
};


#define INADDR_ANY 0

int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);

#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2

int shutdown(int sockfd, int how);

#endif
