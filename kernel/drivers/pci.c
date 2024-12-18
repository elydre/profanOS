#include <ktype.h>
#include <drivers/pci.h>
#include <minilib.h>
#include <cpu/ports.h>
#include <kernel/scubasuit.h>

pci_device_t *pcis = NULL;
int pcis_len = 0;


uint16_t pci_read_config(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    port_long_out(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)((port_long_in(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;

}

void pci_get_ids(pci_device_t *pci) {
	pci->vendor_id = pci_read_config(pci->bus, pci->slot, pci->function, 0);
	if (pci->vendor_id == 0xFFFF)
		return ;
	pci->device_id = pci_read_config(pci->bus, pci->slot, pci->function, 2);
}

void pci_add_device(pci_device_t *pci) {
	pcis = realloc_as_kernel(pcis, (pcis_len + 1) * sizeof(pci_device_t));
	pcis[pcis_len] = *pci;
	pcis_len++;
}

void pci_get_bars(pci_device_t *pci) {
    for (int i = 0; i < 6; i++) {
        uint32_t bar = pci_read_config(pci->bus, pci->slot, pci->function, 0x10 + (i * 4));
        pci->bar[i] = bar;
        pci->bar_is_mem[i] = 0 == (bar & 0x1);
    }
}

void pci_get_class(pci_device_t *pci) {
    uint16_t upper_word = pci_read_config(pci->bus, pci->slot, pci->function, 0x0A); // Bits 31-16
    uint16_t lower_word = pci_read_config(pci->bus, pci->slot, pci->function, 0x08); // Bits 15-0

    uint32_t class_info = ((uint32_t)upper_word << 16) | lower_word; // Combine en un uint32_t

    pci->class_id = (class_info >> 24) & 0xFF;       // Bits 31-2`4

    uint16_t class_code = pci_read_config(pci->bus, pci->slot, pci->function, 0x08);
    pci->subclass_id = (class_code >> 8) & 0xFF;
    pci->prog_if = class_code & 0xFF;
}

int pci_init() {
	for (int i = 0; i < 256; i++) {
		for (int k = 0; k < 16; k++) {
			for (int l = 0; l < 8; l++) {
				pci_device_t pci = {0};
				pci.bus = i;
				pci.slot = k;
				pci.function = l;
				pci_get_ids(&pci);
				if (pci.vendor_id == 0xFFFF)
					continue;
				pci_get_bars(&pci);
				pci_get_class(&pci);
				pci_add_device(&pci);
                pci.interrupt_line = (uint8_t)pci_read_config(pci.bus, pci.slot, pci.function, 0x3C);
                pci.interrupt_pin = (uint8_t)pci_read_config(pci.bus, pci.slot, pci.function, 0x3D);
			}
		}
    }
    return 0;
}

void pci_write_cmd_u8(pci_device_t *pci, uint8_t barN, uint8_t offset, uint8_t value) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map(bar_base + offset, bar_base + offset, 0);
        volatile uint8_t *mem_addr = (volatile uint8_t *)(bar_base + offset);
        *mem_addr = value;
    } else {
        // Si le BAR est de type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        port_byte_out(io_port, value); // Assurez-vous que `outb` est implémentée ou disponible
    }
}

void pci_write_cmd_u16(pci_device_t *pci, uint8_t barN, uint8_t offset, uint8_t value) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map(bar_base + offset, bar_base + offset, 0);
        volatile uint16_t *mem_addr = (volatile uint16_t *)(bar_base + offset);
        *mem_addr = value;
    } else {
        // Si le BAR est de type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        port_word_out(io_port, value); // Assurez-vous que `outb` est implémentée ou disponible
    }
}

void pci_write_cmd_u32(pci_device_t *pci, uint8_t barN, uint8_t offset, uint8_t value) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map(bar_base + offset, bar_base + offset, 0);
        volatile uint32_t *mem_addr = (volatile uint32_t *)(bar_base + offset);
        *mem_addr = value;
    } else {
        // Si le BAR est de type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        port_long_out(io_port, value); // Assurez-vous que `outb` est implémentée ou disponible
    }
}

uint8_t pci_read_cmd_u8(pci_device_t *pci, uint8_t barN, uint8_t offset) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map(bar_base + offset, bar_base + offset, 0);
        return *(volatile uint8_t *)(bar_base + offset);
    } else {
        // Si le BAR est  type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        return port_byte_in(io_port); // Assurez-vous que `outb` est implémentée ou disponible
    }
}

uint16_t pci_read_cmd_u16(pci_device_t *pci, uint8_t barN, uint8_t offset) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map(bar_base + offset, bar_base + offset, 0);
        return *(volatile uint16_t *)(bar_base + offset);
    } else {
        // Si le BAR est  type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        return port_word_in(io_port); // Assurez-vous que `outb` est implémentée ou disponible
    }
}

uint32_t pci_read_cmd_u32(pci_device_t *pci, uint8_t barN, uint8_t offset) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map(bar_base + offset, bar_base + offset, 0);
        return *(volatile uint32_t *)(bar_base + offset);
    } else {
        // Si le BAR est  type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        return port_long_in(io_port); // Assurez-vous que `outb` est implémentée ou disponible
    }
}
