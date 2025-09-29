#include "../mlw_private.h"

uint16_t I_mlw_rand_u16() {
	return rand() * 1.0 / RAND_MAX * 0xFFFF;
}

uint32_t I_mlw_rand_u32() {
	return (uint32_t)rand();
}
