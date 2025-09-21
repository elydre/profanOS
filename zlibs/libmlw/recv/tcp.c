#include "../mlw_private.h"

#include <unistd.h>
int mlw_tcp_recv(mlw_instance_t *inst, void *buffer, int buffer_len, int timeout_ms) {
    if (!inst || !buffer || buffer_len <= 0)
        return 0;

    int received = 0;
    uint8_t *buf_ptr = buffer;
    uint32_t end_time = get_time() + timeout_ms;

    while (received < buffer_len) {
        void *pkt = NULL;
        void *data = NULL;
        int pkt_len = 0, data_len = 0;
        ip_header_t iphdr;

        if (get_time() > end_time)
            break;

        // lit un paquet IP
        if (mlw_ip_recv(&pkt, &pkt_len, &iphdr, &data, &data_len, inst) != 0) {
            usleep(5000); // pause 5ms si aucun paquet
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

        uint32_t seq = ntohl(info.seq);
        uint32_t payload_len = info.len;

        // segment attendu ou plus grand (ACK cumulatif)
        if (seq >= inst->next_packet_seq && payload_len > 0) {
            int copy_len = payload_len;
            if (received + copy_len > buffer_len)
                copy_len = buffer_len - received;

            memcpy(buf_ptr, info.data, copy_len);
            buf_ptr += copy_len;
            received += copy_len;

            // met à jour la séquence attendue
            inst->next_packet_seq = seq + payload_len;

            // envoie l'ACK correspondant
            mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
                                 inst->current_seq, inst->next_packet_seq,
                                 0x10, NULL, 0, inst->window);
        }

        free(pkt);
    }

    return received;
}
