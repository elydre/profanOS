#include "../mlw_private.h"

#include <unistd.h>

int tcp_general_recv(tcp_recv_info_t *info, mlw_instance_t *inst, void **whole) {
    void *pkt = NULL;
    void *data = NULL;
    int pkt_len = 0, data_len = 0;
    ip_header_t iphdr;
    // lit un paquet IP
    int ip_ret = mlw_ip_recv(&pkt, &pkt_len, &iphdr, &data, &data_len, inst);
    if (ip_ret) {
        free(pkt);
        return ip_ret;
    }
    if (iphdr.protocol != 6)
        goto err;
    info->src_ip = iphdr.src_ip;
    info->dest_ip = iphdr.dest_ip;

    if (tcp_get_packet_info(info, data, data_len) != 0)
        goto err;
    if (info->src_port != inst->dest_port || info->dest_port != inst->src_port)
        goto err;
    *whole = pkt;
    return 0;
    err:
        free(pkt);
        return 1;
}
