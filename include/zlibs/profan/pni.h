#ifndef PNI_H
#define PNI_H

#include <stdint.h>
#include <stdlib.h>

#define PNI_ERR_NO_MAC (-1)
#define PNI_ERR_NO_IP (-2)
#define PNI_ERR_NO_ROUTER (-3)
#define PNI_ERR_NO_INIT (-4)
#define PNI_ERR_PACKET_SIZE (-5)
#define PNI_ERR_NO_PACKET (-6)
#define PNI_ERR_FLAGS (-7)

#define PNI_MAX_PACKET 1024

#define PNI_RECV_NOHANG    0b00000000000000000000000000000001
#define PNI_RECV_HANGTIME  0b00000000000000000000000000000010 // incompatible with NOHANG

// return 0 on success
int      pni_init(void);
int      pni_send(uint16_t src_p, uint16_t dest_p, uint8_t *data, size_t data_len, uint8_t *dest_ip);
uint8_t *pni_recv(int *len, uint32_t flags, ...); // *len = error code
void     pni_exit(void);

#endif
