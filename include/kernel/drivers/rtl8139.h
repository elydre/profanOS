#ifndef ETHERNET_H
#define ETHERNET_H

#include <cpu/isr.h>
#include <ktype.h>

void read_mac_addr();
int rtl8139_init();

#endif
