/*****************************************************************************\
|   === socket.c : 2026 ===                                                   |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <sys/socket.h>
#include <errno.h>
#include <modules/socket.h>
#include <stdio.h>

int socket(int domain, int type, int protocol) {
    int ret = socket_socket(domain, type, protocol);

    if (ret >= 0)
        return ret;

    errno = -ret;
    return -1;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int ret = socket_bind(sockfd, addr, addrlen);

    if (ret >= 0)
        return ret;

    errno = -ret;
    return -1;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int ret = socket_connect(sockfd, addr, addrlen);

    if (ret >= 0)
        return ret;

    errno = -ret;
    return -1;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                const struct sockaddr *dest_addr, socklen_t addrlen) {

    sendto_arg_t args = {
        .sockfd = sockfd,
        .buf = buf,
        .len = len,
        .flags = flags,
        .dest_addr = dest_addr,
        .addrlen = addrlen
    };

    int ret = socket_sendto(&args);

    if (ret >= 0)
        return ret;

    errno = -ret;
    return -1;
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    recvfrom_arg_t args = {
        .sockfd = sockfd,
        .buf = buf,
        .len = len,
        .flags = flags,
        .src_addr = src_addr,
        .addrlen = addrlen
    };
    int ret = socket_recvfrom(&args);

    if (ret >= 0)
        return ret;

    errno = -ret;
    return -1;
}
ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    return sendto(sockfd, buf, len, flags, NULL, 0);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return recvfrom(sockfd, buf, len, flags, NULL, NULL);
}
