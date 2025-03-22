/*****************************************************************************\
|   === pci_old.c : 2025 ===                                                  |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/


// #include <drivers/pci.h>
// #include <cpu/ports.h>
// #include <kernel/scubasuit.h>

// uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
//     uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
//     port_long_out(PCI_CONFIG_ADDRESS, address);
//     return port_long_in(PCI_CONFIG_DATA);
// }

// uint16_t pci_config_read_u16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
//     uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
//     port_long_out(PCI_CONFIG_ADDRESS, address);
//     return port_word_in(PCI_CONFIG_DATA);
// }

// // return 0 if not found, 1 if found
// int pci_search_devices(pci_device_t *pci, uint16_t vendor_id, uint16_t device_id) {
//     for (int bus = 0; bus < 256; bus++) {
//         for (int slot = 0; slot < 32; slot++) {
//             pci->vendor_id = pci_config_read(bus, slot, 0, 0) & 0xFFFF;
//             if (pci->vendor_id != vendor_id) continue;
//             pci->exists = 1;
//             pci->device_id = (pci_config_read(bus, slot, 0, 0) >> 16) & 0xFFFF;
//             if (pci->device_id != device_id) continue;
//             pci->bus = bus;
//             pci->slot = slot;
//             for (int i = 0; i < 6; i++) {
//                 pci->bar_type[i] = pci_config_read(bus, slot, 0, 0x10 + i * 4);
//                 pci->bars[i] = pci_config_read(bus, slot, 0, 0x10 + i * 4) & ~0xF;
//             }
//             return 1;
//         }
//     }
//     pci->exists = 0;
//     return 0;
// }

// uint32_t pci_read_u32(pci_device_t *pci, uint8_t bar, uint32_t offset) {
//     if (pci->bar_type[bar] == 0) {
//      scuba_call_map(pci->bars[bar] + offset, pci->bars[bar] + offset, 0);
//         return *(uint32_t *)(pci->bars[bar] + offset);
//     }
//     // read from ports
//     return port_long_in(pci->bars[bar] + offset);
// }

// void pci_write_u32(pci_device_t *pci, uint8_t bar, uint32_t offset, uint32_t value) {
//  if (pci->bar_type[bar] == 0) {
//      scuba_call_map(pci->bars[bar] + offset, pci->bars[bar] + offset, 0);
//      *(uint32_t *)(pci->bars[bar] + offset) = value;
//  }
//  port_long_out(pci->bars[bar] + offset, value);
// }


// uint16_t pci_read_u16(pci_device_t *pci, uint8_t bar, uint32_t offset) {
//     if (pci->bar_type[bar] == 0) {
//      scuba_call_map(pci->bars[bar] + offset, pci->bars[bar] + offset, 0);
//         return *(uint16_t *)(pci->bars[bar] + offset);
//     }
//     // read from ports
//     return port_word_in(pci->bars[bar] + offset);
// }

// void pci_write_u16(pci_device_t *pci, uint8_t bar, uint32_t offset, uint16_t value) {
//  if (pci->bar_type[bar] == 0) {
//      scuba_call_map(pci->bars[bar] + offset, pci->bars[bar] + offset, 0);
//      *(uint16_t *)(pci->bars[bar] + offset) = value;
//  }
//  port_word_out(pci->bars[bar] + offset, value);
//
