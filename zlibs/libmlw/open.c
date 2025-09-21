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
    inst->buffer = NULL;
    inst->buffer_len = 0;
    inst->packets = NULL;
    inst->window = 65535;

    srand(time(NULL));
    inst->current_seq = (uint32_t)rand();

    // send SYN
    if (mlw_tcp_general_send(inst->src_port, dest_ip, dest_port,
                             inst->current_seq, 0, 0x02, NULL, 0, inst->window))
        return -1;

    // wait SYN+ACK
    uint32_t timeout = 10000;
    uint32_t end = get_time() + timeout;

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
                    if ((info.flags & 0x12) == 0x12 && // SYN+ACK
                        info.dest_port == inst->src_port &&
                        info.src_port  == dest_port &&
                        info.ack == inst->current_seq + 1) {

                        inst->next_packet_seq = info.seq + 1;
                        free(pkt);

                        // send final ACK
                        inst->current_seq++;
                        if (mlw_tcp_general_send(inst->src_port, dest_ip, dest_port,
                                                 inst->current_seq, inst->next_packet_seq,
                                                 0x10, NULL, 0, inst->window)) {
							printf("ici \n");
							return -1;
						}

                        return 0; // connected
                    }
                }
            }
            free(pkt);
        }
    }
    return -1; // timeout
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
	res->next_packet_seq = 0;
	res->packets = NULL;
	res->src_port = rand_u16();
	res->dest_port = dest_port;
	res->dest_ip = dest_ip;
	res->buffer = NULL;
	res->buffer_len = 0;
	if (mlw_tcp_connect(res, res->dest_ip, res->dest_port) == -1) {
		syscall_eth_end(res->eth_id);
		free(res);
		return NULL;
	}
	return res;
}