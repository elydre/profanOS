/*****************************************************************************\
|   === lspci.c : 2024 ===                                                    |
|                                                                             |
|    Unix command implementation - list PCI devices                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <profan.h>
#include <string.h>
#include <stdio.h>

/*********************************************************/

typedef struct {
    uint32_t vendor;
    uint32_t device;
    uint32_t class;
    uint32_t subclass;

    uint32_t bus;
    uint32_t slot;
    uint32_t func;

    uint32_t bar[6];
} pci_device_t;

/*********************************************************/

void outportl(uint16_t port, uint32_t data) {
    asm volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}

uint32_t inportl(uint16_t port) {
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/*********************************************************/

pci_device_t **pci_devices = 0;
uint32_t devs = 0;


void add_pci_device(pci_device_t *pdev) {
    pci_devices[devs] = pdev;
    devs++;
    return;
}

uint16_t pci_read_word(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset) {
    outportl (0xCF8, (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | ((uint32_t) 0x80000000));
    return (uint16_t)((inportl (0xCFC) >> ((offset & 2) * 8)) & 0xffff);
}

uint16_t getVendorID(uint16_t bus, uint16_t device, uint16_t function) {
    return pci_read_word(bus, device, function, 0);
}

uint16_t getDeviceID(uint16_t bus, uint16_t device, uint16_t function) {
    return pci_read_word(bus, device, function, 2);
}

uint16_t getClassId(uint16_t bus, uint16_t device, uint16_t function) {
    return (pci_read_word(bus, device, function, 0xA) & ~0x00FF) >> 8;
}

uint16_t getSubClassId(uint16_t bus, uint16_t device, uint16_t function) {
    return (pci_read_word(bus, device, function, 0xA) & ~0xFF00);
}

void pci_probe(void) {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            for (uint32_t function = 0; function < 8; function++) {
                uint16_t vendor = getVendorID(bus, slot, function);
                if (vendor == 0xffff) continue;
                pci_device_t *pdev = malloc(sizeof(pci_device_t));
                pdev->vendor = vendor;
                pdev->device = getDeviceID(bus, slot, function);
                pdev->class = getClassId(bus, slot, function);
                pdev->subclass = getSubClassId(bus, slot, function);
                pdev->bus = bus;
                pdev->slot = slot;
                pdev->func = function;
                for (uint32_t i = 0; i < 6; i++) {
                    pdev->bar[i] = pci_read_word(bus, slot, function, 0x10 + i * 4);
                }
                add_pci_device(pdev);
            }
        }
    }
}

uint16_t pciCheckVendor(uint16_t bus, uint16_t slot) {
    return pci_read_word(bus,slot,0,0);
}

char *get_vendor_name(uint16_t id) {
    switch (id) {
        case 0x1022: return "AMD";
        case 0x10EC: return "Realtek";
        case 0x1234: return "Bochs/QEMU";
        case 0x1AF4: return "VirtIO";
        case 0x1B21: return "ASMedia";
        case 0x1B36: return "Red Hat";
        case 0x1D6B: return "Linux Foundation";
        case 0x8086: return "Intel";
        default:     return "Unknown";
    }
}

char *get_class_name(uint16_t id) {
    switch (id) {
        case 0x00: return "Unknown";
        case 0x01: return "Mass Storage Controller";
        case 0x02: return "Network Controller";
        case 0x03: return "Display Controller";
        case 0x04: return "Multimedia Controller";
        case 0x05: return "Memory Controller";
        case 0x06: return "Bridge Device";
        case 0x07: return "Simple Communication Controller";
        case 0x08: return "Base System Peripheral";
        case 0x09: return "Input Device";
        case 0x0A: return "Docking Station";
        case 0x0B: return "Processor";
        case 0x0C: return "Serial Bus Controller";
        case 0x0D: return "Wireless Controller";
        case 0x0E: return "Intelligent Controller";
        case 0x0F: return "Satellite Communication Controller";
        case 0x10: return "Encryption Controller";
        case 0x11: return "Signal Processing Controller";
        case 0x12: return "Processing Accelerator";
        case 0x13: return "Non-Essential Instrumentation";
        case 0x40: return "Co-Processor";
        case 0xFF: return "Unassigned Class";
        default:   return "Unknown";
    }
}

int main(int argc, char **argv) {
    devs = 0;
    pci_devices = malloc(32 * sizeof(pci_device_t *));
    pci_probe();

    int show_bar = 0;

    if (argc > 1) {
        if (strcmp(argv[1], "-b") == 0) {
            show_bar = 1;
        } else {
            fprintf(stderr, "Usage: lspci [-b]\n");
            return 1;
        }
    }

    for (uint32_t i = 0; i < devs; i++) {
        pci_device_t *pci_dev = pci_devices[i];

        printf("%02x,%02x [%04x:%04x:%x] %s %s (%x %x)\n",
            pci_dev->bus,
            pci_dev->slot,

            pci_dev->vendor,
            pci_dev->device,
            pci_dev->func,

            get_vendor_name(pci_dev->vendor),
            get_class_name(pci_dev->class),

            pci_dev->class,
            pci_dev->subclass
        );

        if (!show_bar)
            continue;
        for (uint32_t j = 0; j < 6; j++)
            printf("BAR%d: %08x ", j, pci_dev->bar[j]);
        puts("\n");
    }
    return 0;
}
