#include "../mlw_private.h"

uint32_t I_mlw_tcp_get_relative(uint32_t seq, uint32_t seq_first) {
	return (seq - seq_first) & 0xFFFFFFFF;
}

uint32_t I_mlw_tcp_has_been_seen(uint32_t seq, uint32_t current_seq, uint32_t seq_first) {
	return I_mlw_tcp_get_relative(seq, seq_first) < I_mlw_tcp_get_relative(current_seq, seq_first);
}
