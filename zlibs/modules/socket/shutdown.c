
#include <errno.h>
#include "udp.h"

ssize_t socket_shutdown(int sockfd, int how) {
    socket_t *sock = socket_find_fd(sockfd);
    protocol_t *prot = socket_find_protocol(sock->type);
    if (!prot || !prot->sendto)
        return -EINVAL;

     return prot->shutdown(sock, how)
}
