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
    printf("PCI init\n");
    pci_init();
    printf("RTL8139 init\n");
    rtl8139_init();
    printf("Read MAC address\n");
    read_mac_address();
}