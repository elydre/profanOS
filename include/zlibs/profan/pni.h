#ifndef PNI_H
#define PNI_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
	uint16_t src_port;
	uint16_t dest_port;
	uint8_t src_ip[4];
	int32_t len; // < 0 -> error
	uint8_t *data;
} pni_packet_t;

#define PNI_ERR_NO_MAC (-1)
#define PNI_ERR_NO_IP (-2)
#define PNI_ERR_NO_ROUTER (-3)
#define PNI_ERR_NO_INIT (-4)
#define PNI_ERR_PACKET_SIZE (-5)
#define PNI_ERR_NO_PACKET (-6)
#define PNI_ERR_FLAGS (-7)

#define PNI_MAX_PACKET 1024

// if need arg they are in order
#define PNI_RECV_NOHANG    0b00000000000000000000000000000001
#define PNI_RECV_HANGTIME  0b00000000000000000000000000000010 // incompatible with NOHANG need arg (int)
#define PNI_RECV_PORT_SRC  0b00000000000000000000000000000100 // need arg (uint32_t)
#define PNI_RECV_PORT_DEST 0b00000000000000000000000000001000 // need arg (uint32_t)

// return 0 on success
int pni_init(void);
int pni_send(uint16_t src_p, uint16_t dest_p, uint8_t *data, size_t data_len, uint8_t *dest_ip);
pni_packet_t pni_recv(uint32_t flags, ...);
void pni_exit(void);
void pni_destroy_packet(pni_packet_t packet);

#endif
