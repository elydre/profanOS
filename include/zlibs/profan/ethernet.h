#ifndef I_ETHERNET_H
#define I_ETHERNET_H

#include <profan/syscall.h>

#define pci_init() c_ethernet_call(0)
#define rtl8139_init() c_ethernet_call(1)
#define read_mac_address() c_ethernet_call(2)

#endif
