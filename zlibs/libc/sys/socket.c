/*****************************************************************************\
|   === socket.c : 2026 ===                                                   |
|                                                                             |
|    Implementation of sys/socket functions from libC              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/socket.h>
#include <profan.h>

#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>

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

int shutdown(int sockfd, int how) {
    int ret = socket_shutdown(sockfd, how);

    if (ret >= 0)
        return ret;

    errno = -ret;
    return -1;
}

int listen(int sockfd, int backlog) {
    return (PROFAN_FNI, -1);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    return (PROFAN_FNI, -1);
}

int socketpair(int domain, int type, int protocol, int sv[2]) {
    return (PROFAN_FNI, -1);
}

#include <string.h>

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    uint32_t val;

    switch (optname) {
        case SO_ERROR:
            val = 0;
            break;
        case SO_REUSEADDR:
            val = 1;
            break;
        case SO_KEEPALIVE:
            val = 0;
            break;
        case SO_TYPE:
            val = 0; // TODO
            break;
    }
    if (optlen && *optlen >= sizeof(uint32_t)) {
        memcpy(optval, &val, sizeof(uint32_t));
        *optlen = sizeof(uint32_t);
    } else if (optlen) {
        *optlen = 0;
    }
    return 0;
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    return 0; // TODO
}

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int ret = socket_getname(sockfd, 1, addr, addrlen);

    if (ret >= 0)
        return ret;

    errno = -ret;
    return -1;
}

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int ret = socket_getname(sockfd, 0, addr, addrlen);

    if (ret >= 0)
        return ret;

    errno = -ret;
    return -1;
}
