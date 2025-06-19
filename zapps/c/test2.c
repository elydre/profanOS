#include <profan/pni.h>
#include <stdio.h>
#include <string.h>

// @LINK: libpni

int main() {
	// Initialize PNI
	if (pni_init() != 0) {
		fprintf(stderr, "PNI initialization failed\n");
		return 1;
	}

	// Prepare message
	const char *message = "aa";
	size_t message_len = strlen(message);
	uint8_t dest_ip[4] = {82, 64, 162, 243}; // Example destination IP

	// Send UDP packet
	if (pni_send(42420, 42420, (uint8_t *)message, message_len, dest_ip) != 0) {
		fprintf(stderr, "Failed to send UDP packet\n");
		pni_exit();
		return 1;
	}

	// Exit PNI
	pni_exit();
	return 0;
}
