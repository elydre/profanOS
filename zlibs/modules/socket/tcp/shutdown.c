/*****************************************************************************\
|   === shutdown.c : 2026 ===                                                 |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "tcp.h"

int socket_tcp_shutdown(socket_t *sock_ptr, int how) {
    tcp_t *sock = sock_ptr->data;
    if (how == SHUT_WR || how == SHUT_RDWR)
        TCP_SET_INFO(sock, TCP_SEND_FIN_MASK);

    if (how == SHUT_RD || how == SHUT_RDWR) {
        free(sock->recv);
        sock->recv = NULL;
        sock->recv_len = 0;
        sock->recv_max = 0;
    }
    return 0;
}
