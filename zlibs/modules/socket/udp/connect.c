#include "udp.h"

int socket_udp_connect(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen) {
    udp_t *data = sock->data;
    if (addrlen != sizeof(struct sockaddr_in))
        return -EINVAL;
    const struct sockaddr_in *addr2 = (void *)addr;
    if (addr2->sin_family != AF_INET)
        return -EINVAL;
    uint16_t port = addr2->sin_port;
    if (port == 0)
        return -EINVAL;
    if (!data->is_bound) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = eth_info.ip;
        addr.sin_port = 0;

        int err = socket_udp_bind(sock, (void *)&addr, sizeof(addr));
        if (err)
            return err;
    }
    data->is_connected = 1;
    data->remote_port = port;
    data->remote_ip = addr2->sin_addr.s_addr;
    return 0;
}
