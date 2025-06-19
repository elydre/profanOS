#include <profan/pni.h>
#include <stdio.h>
#include <string.h>

// @LINK: libpni

int main(int argc, char **argv) {
	(void)argc;

	if (pni_init() != 0) {
		fprintf(stderr, "PNI initialization failed\n");
		return 1;
	}

	const char *message = argv[1];
	size_t message_len = strlen(message);
	uint8_t dest_ip[4] = {82, 64, 162, 243};

	if (pni_send(42420, 42420, (uint8_t *)message, message_len, dest_ip) != 0) {
		fprintf(stderr, "Failed to send UDP packet\n");
		pni_exit();
		return 1;
	}

	int len = 0;
	uint8_t *responde = pni_recv(&len, PNI_RECV_HANGTIME, 500);
	if (len > 0) {
		for (int i = 0; i < len; i++) {
			printf("%c", responde[i]);
		}
		printf("\n");
	}

	pni_exit();
	return 0;
}
