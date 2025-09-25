#include "mlw_private.h"

void mlw_update(mlw_instance_t *inst) {
	if (!inst || !inst->eth_id)
		return ;
	while (1) {
		void *whole = NULL;
		tcp_recv_info_t info;
		int tcp_ret = tcp_general_recv(&info, inst, &whole);
		if (tcp_ret == 2)
			break;
		if (tcp_ret == 1)
			continue;




		free(whole);
	}
}
