#include "stp.h"
#include <profan/pni.h>
#include <profan/syscall.h>
#include <unistd.h>

pni_packet_t stp_recv(uint8_t xid, uint32_t timeout) {
	pni_packet_t resp;
	uint32_t start = syscall_get_time();

	while (start + timeout > syscall_get_time() || timeout == 0) {
		resp = pni_recv(PNI_RECV_NOHANG | PNI_RECV_PORT_SRC | PNI_RECV_PORT_DEST, STP_PORT, STP_PORT);
		if (resp.len < sizeof(stp_header_t)) {
			goto retry;
		}
		stp_header_t *header = (stp_header_t *)resp.data;
		if (header->xid == xid)
			return resp;
		retry:
			free(resp.data);
			resp = (pni_packet_t){0};
			if (timeout == 0)
				break;
			usleep(1000);
	}

	resp.len = -1;
	return resp;
}
