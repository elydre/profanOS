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

#include <cpu/ports.h>
#include <ktype.h>
#include <minilib.h>
#include <drivers/pci.h>
#include <drivers/e1000.h>
#include <cpu/isr.h>

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

void get_mac_address(pci_device_t *pci, uint8_t mac[6]) {
    // addr mac (e1000) at 0x5400
    uint32_t mac_low = pci_read_u32(pci, 0, 0x5400);
    uint16_t mac_high = pci_read_u32(pci, 0, 0x5404) & 0xFFFF;

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
        if (pci.exists && pci.vendor_id == 0x8086 && pci.device_id == 0x100e) {
            found = 1;
            e1000->pci = pci;
            break;
        }
        if (is_last)
            break;
    }
    if (found) {
        get_mac_address(&(e1000->pci), e1000->mac);
        e1000->pci = pci;
        e1000->exists = 1;
        e1000->irq = pci_config_read(pci.bus, pci.slot, 0, 0x3c) & 0xff;
        return ;
    }
}

void e1000_handler(registers_t *regs) {

}


int e1000_init(void) {
    e1000_t device = {0};
    scan_pci_for_e1000(&device);
    if (!device.exists) {
        kprintf("Error: e1000 device noot found\n");
        return 1;
    }
    kprintf("e1000 device found slot %x bus %x\n", device.pci.slot, device.pci.bus);
    kprintf("MAC Address: %x:%x:%x:%x:%x:%x\n",
           device.mac[0], device.mac[1], device.mac[2], device.mac[3], device.mac[4], device.mac[5]);

    uint32_t ctrl = pci_read_u32(&(device.pci), 0,REG_CTRL);
    pci_write_u32(&(device.pci), 0, REG_CTRL, ctrl | (1 << 26));
    while (!(pci_read_u32(&(device.pci), 0, REG_STATUS))) {
        // wait the card to be prepared
    }
    register_interrupt_handler(device.irq + 32, e1000_handler);
    return 0;
}
