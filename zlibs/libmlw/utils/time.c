#include "../mlw_private.h"

uint32_t I_mlw_get_time() {
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
