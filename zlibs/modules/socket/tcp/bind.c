

int socket_tcp_bind(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen) {
    tcp_t *data = sock->data;
    if (addrlen != sizeof(struct sockaddr_in))
        return -EINVAL;
    if (data->is_bound)
        return -EINVAL;
    const struct sockaddr_in *addr2 = (void *)addr;
    if (addr2->sin_family != AF_INET)
        return -EINVAL;
    uint16_t port = addr2->sin_port;
    if (port && !tcp_is_port_free(port))
        return -EADDRINUSE;
    if (port == 0)
        port = tcp_get_free_port();
    if (port == 0)
        return -ENOMEM;
    tcp_lock_port(htons(port));
    data->local_ip = addr2->sin_addr.s_addr;
    data->local_port = port;
    data->is_bound = 1;
    return 0;
}
