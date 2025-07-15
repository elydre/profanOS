#include "stp.h"

uint8_t stp_get_xid() {
	static uint8_t xid = 0;
	return xid++;
}
