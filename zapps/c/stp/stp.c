#include <profan/pni.h>
#include "stp.h"

void stp_uid(uint8_t uid[8]) {
	for (int i = 0; i < 8; i++)
		uid[i] = (uint8_t)(rand() % 256);
}

int main(int argc, char **argv) {
	if (argc == 2)
		return stp_download_url(argv[1]);
	return 0;
}