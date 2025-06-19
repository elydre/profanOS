#include "pni_private.h"
#include <profan/syscall.h>

void pni_exit() {
	syscall_eth_listen_end();
	pni_inited = 0;
}