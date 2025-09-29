#include "../mlw_private.h"

#include <unistd.h>

int I_mlw_tcp_general_recv(tcp_recv_info_t *info, mlw_tcp_t *inst, void **whole) {
    void *pkt = NULL;
    void *data = NULL;
    int pkt_len = 0, data_len = 0;
    ip_header_t iphdr;
    // lit un paquet IP
    int ip_ret = I_mlw_ip_recv(&pkt, &pkt_len, &iphdr, &data, &data_len, inst);
    if (ip_ret) {
        free(pkt);
        return ip_ret;
    }
    if (iphdr.protocol != 6)
        goto err;
    info->src_ip = iphdr.src_ip;
    info->dest_ip = iphdr.dest_ip;
    if (I_mlw_tcp_get_packet_info(info, data, data_len) != 0)
        goto err;
    if (info->src_port != inst->dest_port || info->dest_port != inst->src_port)
        goto err;
    *whole = pkt;
    return 0;
    err:
        free(pkt);
        return 1;
}

int mlw_tcp_recv(mlw_tcp_t *inst, void *buff, int buff_len, int timeout) {
    if (!inst)
        return -1;
    if (!inst->is_open && !inst->recv.buffer_len)
        return -1;
    if (!inst->is_open) {
        int min_len = buff_len;
        if (inst->recv.buffer_len < min_len)
            min_len = inst->recv.buffer_len;
        memcpy(buff, inst->recv.buffer, min_len);
        memmove(inst->recv.buffer, inst->recv.buffer + min_len, inst->recv.buffer_len - min_len);
        inst->recv.buffer_len -= min_len;
        inst->recv.buffer = realloc(inst->recv.buffer, inst->recv.buffer_len);
        return min_len;
    }
    uint32_t end = I_mlw_get_time() + timeout;
    while (1) {
        I_mlw_tcp_update(inst);
        if (inst->recv.buffer_len) {
            int min_len = buff_len;
            if (inst->recv.buffer_len < min_len)
                min_len = inst->recv.buffer_len;
            memcpy(buff, inst->recv.buffer, min_len);
            memmove(inst->recv.buffer, inst->recv.buffer + min_len, inst->recv.buffer_len - min_len);
            inst->recv.buffer_len -= min_len;
            inst->recv.buffer = realloc(inst->recv.buffer, inst->recv.buffer_len);
            return min_len;
        }
        if (timeout < 0)
            continue;
        if (I_mlw_get_time() > end)
            break;
        usleep(800);
    }
    return 0;
}