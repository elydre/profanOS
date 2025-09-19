#include "mlw.h"

eth_info_t *mlw_eth_info = NULL;
uint32_t mlw_eth_id = 0;

void mlw_init() {
	mlw_eth_id = syscall_eth_start();
	if (mlw_eth_id == 0)
		return ;
	mlw_eth_info = malloc(sizeof(eth_info_t));
	syscall_eth_get_info(mlw_eth_id, mlw_eth_info);
}