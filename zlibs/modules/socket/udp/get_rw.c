#include "udp.h"

int socket_udp_get_rw(socket_t *sock) {
    int res = 0;
    udp_t *data = sock->data;
    if (data->recv_len)
        res |= FM_READ;
    if (data->send_len < 64)
        res |= FM_WRITE;
    return res;
}
