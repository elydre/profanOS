#ifndef STP_H
#define STP_H

#include <profan/pni.h>
#include <stdint.h>

typedef struct {
	uint8_t opcode;
	uint8_t xid;
	uint8_t user_id[8];
	uint8_t hashed_key[8];
	uint32_t timestamp;
} __attribute__((packed)) stp_header_t;

void stp_uid(uint8_t uid[8]);
int stp_download_url(const char *url);
int stp_do_read_url(const char *url, uint8_t url_tmp_id[8]);\
uint8_t stp_get_xid();
pni_packet_t stp_recv(uint8_t xid, uint32_t timeout);

#define STP_PORT 42420

#define STP_OPC_READ_URL 0xFF
#define STP_OPC_READ_URL_RESP 0xFE

#endif