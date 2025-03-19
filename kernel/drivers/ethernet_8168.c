#include <cpu/ports.h>
#include <ktype.h>
#include <minilib.h>
#include <drivers/pci.h>
#include <cpu/isr.h>
#include <kernel/snowflake.h>
#include <cpu/timer.h>

static int this_eth_ids[][2] = {
	{0x10ec, 0x8161},
	{0x10ec, 0x8168},
	{0x10ec, 0x8169},
	{0x1259, 0xc107},
	{0x1737, 0x1032},
	{0x16ec, 0x0116},
	{0, 0}
};

int eth_8168_init() {
	pci_device_t *pci = NULL;
	for (int i = 0; this_eth_ids[i][0] != 0; i++) {
		pci = pci_find(this_eth_ids[i][0], this_eth_ids[i][1]);
		if (pci != NULL)
			break;
	}

	if (pci == NULL) {
		return 2;
	}

	kprintf("Realtek 8168 found at %x:%x\n", pci->vendor_id, pci->device_id);
	char mac_address[6] = {0};
	for (int i = 0; i < 6; i++) {
		mac_address[i] = port_byte_in(pci->bar[0] - 1 + i);
	}
	kprintf("MAC Address: %x:%x:%x:%x:%x:%x\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
	uint32_t t0 = timer_get_ms();
	outportb(pci->bar[0] + 0x37, 0x10);
	while((inportb(pci->bar[0] + 0x37) & 0x10) && (t0 + 1000 > timer_get_ms()))
		;

	return 0;
}
