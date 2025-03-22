/*****************************************************************************\
|   === pci_old.h : 2025 ===                                                  |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef PCI_OLD_H
#define PCI_OLD_H

#include <ktype.h>


// IO PCI PORTS
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

typedef struct {
    uint8_t exists;
    uint8_t slot;
    uint8_t bus;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t bar_type[6];
    uint32_t bars[6];
} pci_device_t;


uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
// return 1 if the pci read is the last
int get_next_pci(pci_device_t *pci, int restart);
int pci_search_devices(pci_device_t *pci, uint16_t vendor_id, uint16_t device_id);
uint16_t pci_config_read_u16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);




void pci_write_u32(pci_device_t *pci, uint8_t bar, uint32_t offset, uint32_t value);
uint32_t pci_read_u32(pci_device_t *pci, uint8_t bar, uint32_t offset);
uint16_t pci_read_u16(pci_device_t *pci, uint8_t bar, uint32_t offset);
void pci_write_u16(pci_device_t *pci, uint8_t bar, uint32_t offset, uint16_t value);

#endi
