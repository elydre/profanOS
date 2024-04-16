#include <stdlib.h>
#include <stdio.h>
#include <profan/type.h>
#include <profan/syscall.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>

uint8_t port_byte_in(uint16_t port) {
    uint8_t result;
    asm("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void port_byte_out(uint16_t port, uint8_t data) {
    asm volatile("out %%al, %%dx" : : "a" (data), "d" (port));
}

uint16_t port_word_in(uint16_t port) {
    uint16_t result;
    asm("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void port_word_out(uint16_t port, uint16_t data) {
    asm volatile("out %%ax, %%dx" : : "a" (data), "d" (port));
}

uint32_t port_long_in(uint32_t port) {
    uint32_t result;
    asm volatile("inl %%dx,%%eax":"=a" (result):"d"(port));
    return result;
}

void port_long_out(uint32_t port, uint32_t value) {
    asm volatile("outl %%eax,%%dx"::"d" (port), "a" (value));
}


/*********************************************************/

typedef struct {
    uint16_t device_id;
    uint16_t vendor_id;
    uint16_t status;
    uint16_t command;
    uint8_t class;
    uint8_t subclass;
    uint8_t progif;
    uint8_t revision;
    uint8_t bist;
    uint8_t header_type;
    uint8_t latency_timer;
    uint8_t cache_line_size;
} pci_header_t;

typedef struct {
    pci_header_t *header;
    uint32_t bus;
    uint32_t slot;
    uint32_t func;
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
                pdev->header = malloc(sizeof(pci_header_t));
                pdev->header->vendor_id = vendor;
                pdev->header->device_id = getDeviceID(bus, slot, function);
                pdev->header->class = getClassId(bus, slot, function);
                pdev->header->subclass = getSubClassId(bus, slot, function);
                pdev->bus = bus;
                pdev->slot = slot;
                pdev->func = function;
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

pci_device_t *find_pci_device(uint16_t vendor, uint16_t device) {
    for (uint32_t i = 0; i < devs; i++) {
        if (pci_devices[i]->header->vendor_id == vendor && pci_devices[i]->header->device_id == device) {
            return pci_devices[i];
        }
    }
    return NULL;
}

void main() {
    pci_probe();
    pci_device_t *dev = find_pci_device(0x10EC, 0x8139); // RTL8139
    if (dev == NULL) {
        printf("Device not found\n");
        return;
    }
    // from now on, dev is our network card
    printf("Vendor: %s\n", get_vendor_name(dev->header->vendor_id));
    
    // set ioaddr
    uint32_t ioaddr = 0xC000; // wtf is this?

    // init
    port_byte_out(ioaddr + 0x52, 0x00); // reset
    port_byte_out(ioaddr + 0x37, 0x10);
    while ((port_byte_in(ioaddr + 0x37) & 0x10) != 0) {};

    // init receive buffer
    uint32_t rx_buffer = 0x1000;
    port_word_out(ioaddr + 0x30, (unsigned long int)rx_buffer);
    port_word_out(ioaddr + 0x3C, 0x0005);

    // config of receive buffer
    port_long_out(ioaddr + 0x44, (1 << 7)); // 1 << 7 = WRAP bit

    // enable receive and transmit
    port_byte_out(ioaddr + 0x37, 0x0C); // enable receive and transmit

    
}