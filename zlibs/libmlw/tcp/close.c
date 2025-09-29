#include "../mlw_private.h"

int mlw_tcp_close(mlw_tcp_t *inst) {
    if (!inst)
        return -1;
    if (inst->is_open == 0)
        return 0;


    if (I_mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
        inst->send.next_seq, inst->recv.next_seq, TCP_FLAG_FIN | TCP_FLAG_ACK, NULL, 0, 0xFFFF))
        return -1;

    uint32_t end = I_mlw_get_time() + 5000;

    while (I_mlw_get_time() < end) {
        void *whole = NULL;
        tcp_recv_info_t info;
        int tcp_ret = I_mlw_tcp_general_recv(&info, inst, &whole);
        if (tcp_ret == 2) {
            I_mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
                inst->send.next_seq, inst->recv.next_seq, TCP_FLAG_FIN | TCP_FLAG_ACK, NULL, 0, 0xFFFF);
            usleep(500000);
            continue;
        }
		if (tcp_ret == 1)
			continue;
        if ((info.flags & TCP_FLAG_ACK) == 0) {
            free(whole);
            continue;
        }
        if (info.ack != inst->send.next_seq + 1) {
            free(whole);
            continue;
        }

        inst->send.next_seq++;
    }
    while (I_mlw_get_time() < end) {
        void *whole = NULL;
		tcp_recv_info_t info;
		int tcp_ret = I_mlw_tcp_general_recv(&info, inst, &whole);
		if (tcp_ret == 2) {
            usleep(500000);
            continue;
        }
		if (tcp_ret == 1)
			continue;
        if ((info.flags & TCP_FLAG_FIN) == 0) {
            free(whole);
            continue;
        }
        inst->recv.next_seq++;
        inst->recv.next_seq += info.len;
        while (I_mlw_get_time() < end) {
            if (!I_mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
                inst->send.next_seq, inst->recv.next_seq, TCP_FLAG_ACK, NULL, 0, 0xFFFF)) {
                inst->send.next_seq++;
                break;
            }
            usleep(1500);
        }
    }
    inst->is_open = 0;
    return 0;
}
