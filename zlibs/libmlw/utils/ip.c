#include "../mlw_private.h"

uint32_t mlw_ip_from_u8(uint8_t left, uint8_t mid_left, uint8_t mid_right, uint8_t right) {
	uint32_t res = 0;
	res |= left;
	res |= ((uint32_t)mid_left) << 8;
	res |= ((uint32_t)mid_right) << 16;
	res |= ((uint32_t)right) << 24;
	return res;
}

uint32_t mlw_ip_from_str(char *ip) {
	int dot_count = 0;
	for (int i = 0; ip[i]; i++) {
		if (ip[i] == '.')
			dot_count++;
	}
	if (dot_count != 3)
		return 0;

	uint8_t parts[4] = {0, 0, 0, 0};
	int part_idx = 0;
	int i = 0;
	while (ip[i]) {
		if ('0' <= ip[i] && ip[i] <= '9') {
			parts[part_idx] *= 10;
			parts[part_idx] += ip[i] - '0';
		}
		if (ip[i] == '.')
			part_idx++;
		i++;
	}
	uint32_t res = 0;
	res |= parts[0];
	res |= ((uint32_t)parts[1]) << 8;
	res |= ((uint32_t)parts[2]) << 16;
	res |= ((uint32_t)parts[3]) << 24;
	return res;
}
