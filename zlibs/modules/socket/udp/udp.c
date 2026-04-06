/*****************************************************************************\
|   === udp.c : 2026 ===                                                      |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <sys/socket.h>
#include <minilib.h>
#include <errno.h>

#include "utils.h"
#include "udp.h"
#include "ip.h"

int socket_udp_init(socket_t *sock) {
    udp_t *info = malloc(sizeof(udp_t));
    if (!info)
        return 1;
    sock->data = info;

    info->local_ip = 0;
    info->local_port = 0;
    info->is_bound = 0;
    info->remote_ip = 0;
    info->remote_port = 0;
    info->is_connected = 0;
    info->recv_len = 0;
    info->send_len = 0;

    return 0;
}

void socket_udp_tick(socket_t *sock) {
    if (sock->do_remove)
        return ;
    udp_t *info = sock->data;
    if (!info->send_len && sock->ref_count == 0) {
        udp_free_port(htons(info->local_port));
        info->local_port = 0;
        sock->do_remove = 1;
        free(info);
        sock->data = NULL;
        sock->type = 0;
        return ;
    }
    if (!info->send_len)
        return ;
    socket_on_send_udp(&info->send[0]);
    free(info->send[0].data);
    info->send_len--;
	mem_copy(info->send, &info->send[1], sizeof(udp_packet_t) * info->send_len);
}
