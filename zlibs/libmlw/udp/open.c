#include "../mlw_private.h"

mlw_udp_t *mlw_udp_open(uint32_t dest_ip, uint16_t dest_port) {
	mlw_udp_t *res = malloc(sizeof(mlw_udp_t));
	if (!res)
		return NULL;
	res->dest_ip = dest_ip;
	res->dest_port = dest_port;
	res->eth_id = syscall_eth_start();
	if (!res->eth_id) {
		free(res);
		return NULL;
	}
	res->src_port = I_mlw_rand_u16();
	return res;
}