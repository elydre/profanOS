#include "mlw_private.h"

int mlw_tcp_close(mlw_instance_t *inst) {
    if (!inst)
        return -1;
    if (inst->is_open == 0)
        return 0;


    if (mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
        inst->send.next_seq, inst->recv.next_seq, TCP_FLAG_FIN | TCP_FLAG_ACK, NULL, 0, 0xFFFF))
        return -1;

    uint32_t end = get_time() + 5000;

    while (get_time() < end) {
        void *whole = NULL;
        tcp_recv_info_t info;
        int tcp_ret = tcp_general_recv(&info, inst, &whole);
        if (tcp_ret == 2) {
            mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
                inst->send.next_seq, inst->recv.next_seq, TCP_FLAG_FIN | TCP_FLAG_ACK, NULL, 0, 0xFFFF);
            usleep(500);
            continue;
        }
		if (tcp_ret == 1)
			continue;
        if ((info.flags & TCP_FLAG_ACK) == 0) {
            free(whole);
            continue;
        }
        if (info.ack != inst->send.next_seq + 1)
            free(whole);
            continue;

        inst->send.next_seq++;
    }
    while (get_time() < end) {
        void *whole = NULL;
		tcp_recv_info_t info;
		int tcp_ret = tcp_general_recv(&info, inst, &whole);
		if (tcp_ret == 2) {
            usleep(500);
            continue;
        }
		if (tcp_ret == 1)
			continue;
        if ((info.flags & TCP_FLAG_FIN) == 0) {
            free(whole);
            continue;
        }
        inst->recv.next_seq++;
        while (get_time() < end) {
            if (!mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
                inst->send.next_seq, inst->recv.next_seq, TCP_FLAG_ACK, NULL, 0, 0xFFFF)) {
                inst->send.next_seq++;
                break;
            }
            usleep(500);
        }
    }
    inst->is_open = 0;
    return 0;
}
