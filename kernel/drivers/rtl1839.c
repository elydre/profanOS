/*****************************************************************************\
|   === rtl1839.c : 2025 ===                                                  |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <drivers/pci.h>
#include <minilib.h>
#include <cpu/timer.h>

int rtl8139_init() {
	pci_device_t *pci = pci_find(0x10ec, 0x8139);
	if (pci == NULL) {
		return 2;
	}
	kprintf("Realtek 8139 found at %x:%x:%x\n", pci->vendor_id, pci->device_id, pci->bar[0]);
	uint8_t mac[6] = {0};
	for (int i = 0; i < 6; i++) {
		mac[i] = pci_read_cmd_u8(pci, 0, i);
	}
	kprintf("MAC Address: %x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	pci_write_cmd_u32(pci, 0, 0x52, 0x0000);// turn it on
	// do software reset
	pci_write_cmd_u8(pci, 0, 0x37, 0x10);
	uint32_t t0 = timer_get_ms();
	while ((pci_read_cmd_u8(pci, 0, 0x37) & 0x10) != 0 && timer_get_ms() < t0 + 1000) {
		// wait for reset to complete
	}
	uint32_t t1 = timer_get_ms();
	kprintf("ISR = %x\n", pci_read_cmd_u16(pci, 0, 0x3e));
	return 0;
}