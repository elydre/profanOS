#include "../mlw_private.h"

#include <unistd.h>

void *mlw_tcp_recv(mlw_instance_t *inst, int *buffer_len, int timeout_ms) {
    if (!inst || !buffer_len)
        return NULL;

    uint32_t end_time = get_time() + timeout_ms;

    while (1) {
        void *pkt = NULL;
        void *data = NULL;
        int pkt_len = 0, data_len = 0;
        ip_header_t iphdr;

        // lit un paquet IP
        if (mlw_ip_recv(&pkt, &pkt_len, &iphdr, &data, &data_len, inst) != 0) {
            usleep(5000); // 5ms pause
            continue;
        }

        if (iphdr.protocol != 6) { // pas TCP
            free(pkt);
            continue;
        }

        tcp_recv_info_t info;
        info.src_ip = iphdr.src_ip;
        info.dest_ip = iphdr.dest_ip;

        if (tcp_get_packet_info(&info, data, data_len) != 0) {
            free(pkt);
            continue;
        }

        // vérifie que c’est bien le serveur attendu
        if (info.src_port != inst->dest_port || info.dest_port != inst->src_port) {
            free(pkt);
            continue;
        }

        uint32_t seq = info.seq;
        uint32_t payload_len = info.len;

        // si c'est le segment attendu avec données
        if (seq == inst->next_packet_seq && payload_len > 0) {
            // alloue un buffer pour ce segment
            uint8_t *buf = malloc(payload_len);
            if (!buf) {
                free(pkt);
                return NULL;
            }
            memcpy(buf, info.data, payload_len);
            *buffer_len = payload_len;

            // met à jour la séquence attendue
            inst->next_packet_seq = seq + payload_len;

            // envoie l'ACK correspondant
            mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
                                 inst->current_seq, inst->next_packet_seq,
                                 0x10, NULL, 0, inst->window);

            free(pkt);
            return buf;
        }

        free(pkt);
        if (timeout_ms < 0)
            continue;
        if (get_time() >= end_time)
            break;
    }

    return NULL; // timeout
}
