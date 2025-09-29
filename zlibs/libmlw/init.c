#include "mlw_private.h"

eth_info_t *mlw_info = NULL;

int mlw_init(uint32_t flags) {
	if (flags & MLW_INIT_RAND)
		srand(time(NULL));
	mlw_info = malloc(sizeof(eth_info_t));
	if (!mlw_info)
		return 1;
	syscall_eth_get_info(0, mlw_info);
	return 0;
}