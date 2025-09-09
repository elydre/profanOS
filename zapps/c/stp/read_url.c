#include "stp.h"
#include <stdio.h>
#include <stdlib.h>
#include <profan/pni.h>
#include <profan/syscall.h>
#include <string.h>
#include <unistd.h>

int stp_do_read_url(const char *url, uint8_t url_tmp_id[8]) {
	uint8_t *packet = malloc(sizeof(stp_header_t) + 2 + strlen(url));
	uint8_t xid = stp_get_xid();

	stp_header_t *header = (stp_header_t *)packet;
	header->opcode = STP_OPC_READ_URL;
	header->xid = xid;

	uint8_t *data = packet + sizeof(stp_header_t);
	int url_len = strlen(url);
	if (url_len > 0xFFFF)
		return free(packet), 1;

	data[0] = (((uint32_t)url_len) >> 8) & 0xFF;
	data[1] = ((uint32_t)url_len) & 0xFF;
	memcpy(data + 2, url, url_len);

	if (pni_send(STP_PORT, STP_PORT, packet, sizeof(stp_header_t) + 2 + url_len, (uint8_t [4]) {82, 64, 162, 243}))
		return free(packet), 1;
	free(packet);

	pni_packet_t resp = stp_recv(xid, 1000);
	stp_header_t *resp_header = (stp_header_t *)resp.data;
	if (resp.len == -1 || resp.len < (int) sizeof(stp_header_t) + 1 + 8)
		return 1;


	if (resp_header->opcode != STP_OPC_READ_URL_RESP)
		return pni_destroy_packet(resp), 1;
	if (resp.data[sizeof(stp_header_t)] != 0)
		return pni_destroy_packet(resp), 1;

	memcpy(url_tmp_id, &(resp.data[sizeof(stp_header_t) + 1]), 8);
	free(resp.data);
	return 0;
}
