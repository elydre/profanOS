#include <profan/pni.h>

void pni_destroy_packet(pni_packet_t packet) {
	free(packet.data);
}
