/*****************************************************************************\
|   === pci.h : 2025 ===                                                      |
|                                                                             |
|    Kernel PCI driver header                                      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef PCI_H
#define PCI_H

#include <ktype.h>

typedef struct {
    int vendor_id;
    int device_id;
    int bus;
    int slot;
    int function;
    uint8_t bar_is_mem[6];
    uint32_t bar[6];
    uint32_t class_id;
    uint8_t subclass_id;
    uint8_t prog_if;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
} pci_device_t;

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
} pci_findme_t;

extern pci_device_t *pcis;
extern int pcis_len;

void     pci_write_config(pci_device_t *pci, uint8_t offset, uint32_t value);
void     pci_write_config_u16(pci_device_t *pci, uint8_t offset, uint16_t value);
uint32_t pci_read_config(pci_device_t *pci, uint32_t offset);
uint16_t pci_read_config_u16(pci_device_t *pci, uint8_t offset);

uint8_t  pci_read_cmd_u8(pci_device_t *pci, uint8_t bar, uint32_t offset);
uint16_t pci_read_cmd_u16(pci_device_t *pci, uint8_t bar, uint32_t offset);
uint32_t pci_read_cmd_u32(pci_device_t *pci, uint8_t bar, uint32_t offset);
void     pci_write_cmd_u8(pci_device_t *pci, uint8_t bar, uint32_t offset, uint8_t value);
void     pci_write_cmd_u16(pci_device_t *pci, uint8_t bar, uint32_t offset, uint16_t value);
void     pci_write_cmd_u32(pci_device_t *pci, uint8_t bar, uint32_t offset, uint32_t value);

pci_device_t *pci_find(uint16_t vendor, uint16_t device);
pci_device_t *pci_find_array(pci_findme_t *ids, int count);

void     pci_enable_bus_master(pci_device_t *pci);
uint32_t pci_try_enable_msi(pci_device_t *pci);

int pci_init();

#endif
