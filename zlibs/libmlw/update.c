#include "mlw_private.h"

void mlw_update(mlw_instance_t *inst) {
	if (!inst || !inst->eth_id)
		return ;
	while (1) {
		void *whole = NULL;
		int whole_len = 0;
		ip_header_t header;
		void *ip_data = NULL;
		int ip_data_len = 0;
		int ip_ret = mlw_ip_recv(&whole, &whole_len, &header, &ip_data, &ip_data_len, inst);
		if (ip_ret == 2)
			break;
		if (ip_ret == 1)
			continue;
		if (header.src_ip != inst->dest_ip || header.dest_ip != mlw_info->ip)
			goto end_loop;
		if (header.protocol != 6 || !ip_data || ip_data_len <= 0)
			goto end_loop;

		tcp_recv_info_t tcp_info;
		if (tcp_get_packet_info(&tcp_info, ip_data, ip_data_len))
			goto end_loop;
		if (tcp_info.dest_port != inst->src_port || tcp_info.src_port != inst->dest_port)
			goto end_loop;




		end_loop:
			free(whole);
			continue;
	}
}
