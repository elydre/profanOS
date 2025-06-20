#include "stp.h"
#include <stdio.h>
#include <stdlib.h>
#include <profan/pni.h>
#include <profan/syscall.h>
#include <string.h>
#include <unistd.h>

int stp_do_read_url(const char *url, uint8_t url_tmp_id[8]) {
	uint8_t *packet = malloc(sizeof(stp_header_t) + 2 + strlen(url));

	stp_header_t *header = (stp_header_t *)packet;
	header->opcode = STP_OPC_READ_URL;

	uint8_t *data = packet + sizeof(stp_header_t);
	int url_len = strlen(url);
	if (url_len > 0xFFFF) {
		free(packet);
		return 1;
	}
	data[0] = (((uint32_t)url_len) >> 8) & 0xFF;
	data[1] = ((uint32_t)url_len) & 0xFF;
	memcpy(data + 2, url, url_len);

	if (pni_send(STP_PORT, STP_PORT, packet, sizeof(stp_header_t) + 2 + url_len, (uint8_t [4]) {82, 64, 162, 243})) {
		free(packet);
		return 1;
	}
	free(packet);
	uint32_t time_start = syscall_timer_get_ms();
	while (1) {
		if (syscall_timer_get_ms() - time_start > 500)
			return 1;
		usleep(1000);
		pni_packet_t resp = pni_recv(PNI_RECV_NOHANG);
		if (resp.len == 0 || resp.data == NULL)
			return 1;
		if (resp.dest_port != STP_PORT || resp.src_port != STP_PORT) {
			free(resp.data);
			continue;
		}
		if (resp.len < (int)sizeof(stp_header_t) + 9) {
			free(resp.data);
			continue;
		}

		stp_header_t *resp_header = (stp_header_t *)resp.data;
		uint8_t *resp_data = resp.data + sizeof(stp_header_t);
		if (resp_header->opcode != STP_OPC_READ_URL_RESP) {
			free(resp.data);
			continue;
		}
		if (resp_data[0] != 0) {
			free(resp.data);
			continue;
		}
		memcpy(url_tmp_id, resp_data + 1, 8);
		free(resp.data);
		return 0;
	}
	return 0;
}
