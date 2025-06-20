
#include "stp.h"
#include <profan/pni.h>
#include <stdio.h>

int stp_download_url(const char *url) {
	uint8_t url_tmp_id[8];

	if (pni_init()) {
		fprintf(stderr, "Failed to initialize PNI\n");
		return 1;
	}
	if (stp_do_read_url(url, url_tmp_id)) {
		fprintf(stderr, "Failed to read URL (server denied): %s\n", url);
		pni_exit();
		return 1;
	}


	pni_exit();
	return 0;
}