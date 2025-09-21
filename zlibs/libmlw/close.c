#include "mlw_private.h"

int mlw_tcp_close(mlw_instance_t *inst, int timeout_ms) {
    if (!inst)
        return -1;

    // 1️⃣ Envoie FIN + ACK
    if (mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
                             inst->current_seq, inst->next_packet_seq,
                             0x11, NULL, 0, inst->window)) // FIN + ACK
        return -1;

    inst->current_seq++; // FIN consomme 1 seq

    uint32_t end = get_time() + timeout_ms;

    int fin_ack_received = 0;
    int fin_from_server = 0;

    while (get_time() < end) {
        void *pkt, *data;
        int pkt_len, data_len;
        ip_header_t iphdr;

        if (mlw_ip_recv(&pkt, &pkt_len, &iphdr, &data, &data_len, inst) == 0) {
            if (iphdr.protocol == 6) {
                tcp_recv_info_t info;
                info.dest_ip = iphdr.dest_ip;
                info.src_ip  = iphdr.src_ip;

                if (!tcp_get_packet_info(&info, data, data_len)) {
                    if (info.dest_port == inst->src_port &&
                        info.src_port  == inst->dest_port) {

                        // ACK de notre FIN
                        if ((info.flags & 0x10) && info.ack == inst->current_seq)
                            fin_ack_received = 1;

                        // FIN du serveur
                        if (info.flags & 0x01) {
                            fin_from_server = 1;
                            inst->next_packet_seq = info.seq + 1;

                            // envoyer ACK pour le FIN serveur
                            mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
                                                 inst->current_seq, inst->next_packet_seq,
                                                 0x10, NULL, 0, inst->window);
                        }

                        if (fin_ack_received && fin_from_server) {
                            free(pkt);
                            return 0; // fermeture complète
                        }
                    }
                }
            }
            free(pkt);
        }
    }

    // timeout
    return -1;
}
