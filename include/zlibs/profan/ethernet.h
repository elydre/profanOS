#ifndef I_ETHERNET_H
#define I_ETHERNET_H

#include <profan/syscall.h>

#define read_mac_address() c_ethernet_call(0)

#endif
