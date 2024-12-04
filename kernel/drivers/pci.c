
#include <drivers/pci.h>
#include <cpu/ports.h>
#include <kernel/scubasuit.h>

uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    port_long_out(PCI_CONFIG_ADDRESS, address);
    return port_long_in(PCI_CONFIG_DATA);
}

// return 1 if the pci read is the last
int get_next_pci(pci_device_t *pci, int restart) {
    static int slot = 0;
    static int bus = 0;

    if (restart) {
        slot = 0;
        bus = 0;
    }

    // reset infos
    for (uint32_t i = 0; i < sizeof(pci_device_t); i++)
        ((uint8_t *)pci)[i] = 0;

    pci->slot = slot;
    pci->bus = bus;

    pci->vendor_id = pci_config_read(bus, slot, 0, 0) & 0xFFFF;
    if (pci->vendor_id == 0xFFFF) {
        slot++;
        if (slot == 32) {
            slot = 0;
            bus++;
            if (bus == 0) {
                return 1;
            }
        }
        return 0;
    }
    pci->exists = 1;
    for (uint32_t i = 0; i < 6; i++) {
        pci->bar_type[i] = pci_config_read(bus, slot, 0, 0x10 + i * 4);
		pci->bars[i] = pci_config_read(bus, slot, 0, 0x10 + i * 4) & ~0xF;
    }
    pci->device_id = (pci_config_read(bus, slot, 0, 0) >> 16) & 0xFFFF;
    slot++;
    if (slot == 32) {
        slot = 0;
        bus++;
        if (bus == 0) {
            return 1;
        }
    }
    return 0;
}

uint32_t pci_read_u32(pci_device_t *pci, uint8_t bar, uint32_t offset) {
    if (pci->bar_type[bar] == 0) {
		scuba_call_map(pci->bars[bar] + offset, pci->bars[bar] + offset, 0);
        return *(uint32_t *)(pci->bars[bar] + offset);
    }
    // read from ports
    return port_long_in(pci->bars[bar] + offset);
}

void pci_write_u32(pci_device_t *pci, uint8_t bar, uint32_t offset, uint32_t value) {
	if (pci->bar_type[bar] == 0) {
		scuba_call_map(pci->bars[bar] + offset, pci->bars[bar] + offset, 0);
		*(uint32_t *)(pci->bars[bar] + offset) = value;
	}
	port_long_out(pci->bars[bar] + offset, value);
}


uint16_t pci_read_u16(pci_device_t *pci, uint8_t bar, uint32_t offset) {
    if (pci->bar_type[bar] == 0) {
		scuba_call_map(pci->bars[bar] + offset, pci->bars[bar] + offset, 0);
        return *(uint16_t *)(pci->bars[bar] + offset);
    }
    // read from ports
    return port_word_in(pci->bars[bar] + offset);
}

void pci_write_u16(pci_device_t *pci, uint8_t bar, uint32_t offset, uint16_t value) {
	if (pci->bar_type[bar] == 0) {
		scuba_call_map(pci->bars[bar] + offset, pci->bars[bar] + offset, 0);
		*(uint16_t *)(pci->bars[bar] + offset) = value;
	}
	port_word_out(pci->bars[bar] + offset, value);
}