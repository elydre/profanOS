/*****************************************************************************\
|   === e1000.c : 2024 ===                                                    |
|                                                                             |
|    Implementation of e1000 network card driver                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/scubasuit.h>
#include <cpu/ports.h>
#include <ktype.h>
#include <minilib.h>

uint32_t mmio_read32(void *addr) {
    scuba_call_map(addr, addr, 1);
    return *(volatile uint32_t *) addr;
}

//----------------------------------------------------//
//                        PCI                         //
//----------------------------------------------------//

// Ports E/S PCI
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

typedef struct {
    uint8_t exists;
    uint8_t slot;
    uint8_t bus;
    uint16_t vendor_id;
    uint16_t device_id;
} pci_device_t;


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
    for (uint32_t i = 0; i < sizeof(pci); i++)
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

//----------------------------------------------------//

typedef struct {
    uint8_t exists;
    uint8_t mac[6];
    pci_device_t pci;
    uint8_t irq;
} e1000_t;

uint32_t endian_switch_u32(uint32_t x) {
    uint32_t res = 0;
    res |= (x & 0xff) << 24 ;
    res |= ((x >> 8) & 0xff) << 16;
    res |= ((x >> 16) & 0xff) << 8;
    res |= ((x >> 24) & 0xff);
    return res;
}

uint16_t endian_switch_u16(uint16_t x) {
    return ((x & 0xff) << 8) | ((x >> 8) & 0xff);
}

void get_mac_address(uint32_t bar0, uint8_t mac[6]) {
    // addr mac (e1000) at 0x5400
    uint32_t mac_low = mmio_read32((void *)(bar0 + 0x5400));
    uint16_t mac_high = mmio_read32((void *)(bar0 + 0x5404)) & 0xFFFF;

    mac[0] = mac_low & 0xFF;
    mac[1] = (mac_low >> 8) & 0xFF;
    mac[2] = (mac_low >> 16) & 0xFF;
    mac[3] = (mac_low >> 24) & 0xFF;
    mac[4] = mac_high & 0xFF;
    mac[5] = (mac_high >> 8) & 0xFF;


}

void scan_pci_for_e1000(e1000_t *e1000) {
    pci_device_t pci;
    int found  = 0;
    while (1) {
        int is_last = get_next_pci(&pci, 0);
        kprintf("is%d\n", is_last);
        if (pci.exists && pci.vendor_id == 0x8086 && pci.device_id == 0x100e) {
            found = 1;
            e1000->pci = pci;
            break;
        }
        if (is_last)
            break;
    }
    if (found) {
        uint32_t bar0 = pci_config_read(pci.bus, pci.slot, 0, 0x10) & ~0xF;
        kprintf("BAR0 (Base Address Register) = %x %d %d\n", bar0 + 0x5400, pci.bus, pci.slot);

        get_mac_address(bar0, e1000->mac);
        e1000->pci = pci;
        e1000->exists = 1;
        e1000->irq = pci_config_read(pci.bus, pci.slot, 0, 0x3c) & 0xff;
        return ;
    }
}

int e1000_init(void) {
    e1000_t device = {0};
    scan_pci_for_e1000(&device);
    if (!device.exists) {
        kprintf("Error: e1000 device noot found\n");
        return 1;
    }
    kprintf("MAC Address: %x:%x:%x:%x:%x:%x\n",
           device.mac[0], device.mac[1], device.mac[2], device.mac[3], device.mac[4], device.mac[5]);
    return 0;
}
