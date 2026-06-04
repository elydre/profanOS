/*****************************************************************************\
|   === getnames.c : 2026 ===                                                 |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <errno.h>
#include "udp.h"

int socket_udp_getname(socket_t *sock, int local, struct sockaddr *addr, socklen_t *addrlen) {
    udp_t *data = sock->data;
    if (*addrlen < sizeof(struct sockaddr_in)) {
        *addrlen = sizeof(struct sockaddr_in);
        return -EINVAL;
    }
    struct sockaddr_in *in_addr = (struct sockaddr_in *)addr;
    in_addr->sin_family = AF_INET;

    if (local) {
         in_addr->sin_port = data->local_port;
         in_addr->sin_addr.s_addr = data->local_ip;
    } else {
         in_addr->sin_port = data->remote_port;
         in_addr->sin_addr.s_addr = data->remote_ip;
    }

    *addrlen = sizeof(struct sockaddr_in);
    return 0;
}
