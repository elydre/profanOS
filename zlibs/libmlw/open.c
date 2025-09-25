#include "mlw_private.h"
#include <stdlib.h>
#include <sys/time.h>

static int is_init_rand = 0;

uint16_t rand_u16() {
	if (!is_init_rand) {
		is_init_rand = 1;
		srand(time(NULL));
	}
	return rand() * 1.0 / RAND_MAX * 0xFFFF;
}

uint32_t rand_u32() {
	if (!is_init_rand) {
		is_init_rand = 1;
		srand(time(NULL));
	}
	return (uint32_t)rand();
}

uint32_t get_time() {
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int tcp_get_packet_info(tcp_recv_info_t *info, uint8_t *packet, int len) {
    if (len < (int)sizeof(tcp_header_t))
        return 1;

    tcp_header_t *header = (void *)packet;

    info->dest_port = ntohs(header->dst_port);
    info->src_port  = ntohs(header->src_port);
    info->seq       = ntohl(header->seq_num);
    info->ack       = ntohl(header->ack_num);
    info->flags     = header->flags;
    info->window    = ntohs(header->window);

    info->data = packet + sizeof(tcp_header_t);
    info->len  = len - sizeof(tcp_header_t);

    return 0;
}

eth_info_t *mlw_info = NULL;
// init TCP connection (client side)
int mlw_tcp_connect(mlw_instance_t *inst, uint32_t dest_ip, uint16_t dest_port) {
    // init state
    inst->dest_ip = dest_ip;
    inst->dest_port = dest_port;
    inst->is_open = 0;
    inst->is_waiting_seq = 0;
    inst->recv.buffer = NULL;
    inst->recv.buffer_len = 0;
    inst->recv.first_seq = 0;
    inst->recv.next_seq = 0;
    inst->src_port = rand_u16();

    inst->send.first_seq = (uint32_t)rand();
    inst->send.next_seq = inst->send.first_seq;

    // send SYN
    if (mlw_tcp_general_send(inst->src_port, dest_ip, dest_port,
                             inst->send.next_seq, 0, 0x02, NULL, 0, 0xffff))
        return -1;
    inst->send.next_seq++;

    uint32_t timeout = 5000;
    uint32_t end = get_time() + timeout;
    while (get_time() < end) {
        // wait for syn ack
        void *whole = NULL;
        tcp_recv_info_t info;
        int tcp_ret = tcp_general_recv(&info, inst, &whole);
        if (tcp_ret == 2) {
            usleep(500);
            continue;
        }
        if (tcp_ret == 1)
            continue;

        if (info.src_ip != dest_ip || info.dest_port != inst->src_port
                || info.dest_ip != mlw_info->ip || info.src_port != inst->dest_port)
            goto err;
        if ((info.flags & TCP_FLAG_ACK) == 0 || (info.flags & TCP_FLAG_SYN) == 0)
            goto err;
        if (info.ack != inst->send.next_seq)
            goto err;

        inst->recv.next_seq = info.seq + 1;
        inst->recv.first_seq = info.seq;

        // send ack for server syn+ack
        if (mlw_tcp_general_send(inst->src_port, inst->dest_ip, inst->dest_port,
                inst->send.next_seq, inst->recv.next_seq, TCP_FLAG_ACK, NULL, 0, 0xFFFF)) {
            free(whole);
            return -1;
        }
        inst->is_open = 1;
        return 0;
        err:
            free(whole);
            continue;
    }
    return -1;
}

mlw_instance_t *mlw_open(uint32_t dest_ip, uint16_t dest_port) {
	if (mlw_info == NULL) {
		mlw_info = malloc(sizeof(eth_info_t));
		syscall_eth_get_info(0, mlw_info);
	}
	if (mlw_info == NULL)
		return NULL;
	mlw_instance_t *res = malloc(sizeof(mlw_instance_t));
	if (!res)
		return NULL;
	res->eth_id = syscall_eth_start();
	if (res->eth_id == 0) {
		free(res);
		return NULL;
	}
	if (mlw_tcp_connect(res, res->dest_ip, res->dest_port) == -1) {
		syscall_eth_end(res->eth_id);
		free(res);
		return NULL;
	}
	return res;
}