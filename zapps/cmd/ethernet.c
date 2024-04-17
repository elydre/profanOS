#include <stdlib.h>
#include <stdio.h>
#include <profan/type.h>
#include <profan/syscall.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>
#include <string.h>

#include <profan/ethernet.h>

void main() {
    printf("Read MAC address\n");
    read_mac_address();
}