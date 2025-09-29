#include "../mlw_private.h"

static void close_self(mlw_tcp_t *inst) {
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
}

void I_mlw_tcp_update(mlw_tcp_t *inst) {
	if (!inst || !inst->eth_id  || !inst->is_open)
		return ;
	while (1) {
		void *whole = NULL;
		tcp_recv_info_t info;
		int tcp_ret = I_mlw_tcp_general_recv(&info, inst, &whole);
		if (tcp_ret == 2)
			break;
		if (tcp_ret == 1)
			continue;
		if ((info.flags & TCP_FLAG_ACK) != 0 && inst->is_waiting_seq) {
			if (info.ack == inst->seq_to_wait)
				inst->is_waiting_seq = 0;
		}
		if ((info.flags & TCP_FLAG_FIN) != 0) {
			int tries = 5;
			while (tries--) {
				if (I_mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
					inst->send.next_seq, inst->recv.next_seq + 1 + info.len, TCP_FLAG_ACK, NULL, 0, 0xFFFF)) {
						usleep(500);
						continue;
					}
				break;
			}
			close_self(inst);
			inst->is_open = 0;
		}
		if (info.data && info.len > 0) {
			if (info.seq == inst->recv.next_seq) {
				inst->recv.next_seq += info.len;
				I_mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
					inst->send.next_seq, inst->recv.next_seq, TCP_FLAG_ACK, NULL, 0, 0xFFFF);
				inst->recv.buffer = realloc(inst->recv.buffer, inst->recv.buffer_len + info.len);
				memcpy(inst->recv.buffer + inst->recv.buffer_len, info.data, info.len);
				inst->recv.buffer_len += info.len;
			}
			else if (I_mlw_tcp_has_been_seen(info.seq, inst->recv.next_seq, inst->recv.first_seq))
				I_mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
					inst->send.next_seq, inst->recv.next_seq, TCP_FLAG_ACK, NULL, 0, 0xFFFF);
		}
 		free(whole);
	}
}
