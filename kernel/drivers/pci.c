/*****************************************************************************\
|   === pci.c : 2025 ===                                                      |
|                                                                             |
|    Kernel PCI driver implementation                              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <ktype.h>
#include <drivers/pci.h>
#include <minilib.h>
#include <cpu/ports.h>
#include <kernel/scubasuit.h>

#define PCI_CAP_ID_MSI 0x05
#define IA32_APIC_BASE_MSR 0x1B
#define LAPIC_DEFAULT_BASE 0xFEE00000

pci_device_t *pcis = NULL;
int pcis_len = 0;

void pci_write_config(pci_device_t *pci, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31)
                     | (pci->bus << 16)
                     | (pci->slot << 11)
                     | (pci->function << 8)
                     | (offset & 0xFC);

    port_write32(0xCF8, address);
    port_write32(0xCFC, value);
}

void pci_write_config_u16(pci_device_t *pci, uint8_t offset, uint16_t value) {
    uint32_t shift = (offset & 0x3) * 8;
    uint32_t address = (1 << 31)
                     | (pci->bus << 16)
                     | (pci->slot << 11)
                     | (pci->function << 8)
                     | (offset & 0xFC);

    port_write32(0xCF8, address);

    uint32_t existing = port_read32(0xCFC);
    existing &= ~(0xFFFF << shift);
    existing |= ((uint32_t)value) << shift;

    port_write32(0xCFC, existing);
}

uint32_t pci_read_config(pci_device_t *pci, uint32_t offset) {
    uint32_t address = (1 << 31)
                     | (pci->bus << 16)
                     | (pci->slot << 11)
                     | (pci->function << 8)
                     | (offset & 0xFC)
                     | ((uint32_t) 0x80000000);

    port_write32(0xCF8, address);
    return port_read32(0xCFC);
}

uint16_t pci_read_config_u16(pci_device_t *pci, uint8_t offset) {
    uint32_t shift = (offset & 0x3) * 8;
    uint32_t address = (1 << 31)
                     | (pci->bus << 16)
                     | (pci->slot << 11)
                     | (pci->function << 8)
                     | (offset & 0xFC);

    port_write32(0xCF8, address);
    return (port_read32(0xCFC) >> shift) & 0xFFFF;
}

static void pci_get_ids(pci_device_t *pci) {
    pci->vendor_id = pci_read_config(pci, 0) & 0xffff;
    if (pci->vendor_id == 0xFFFF)
        return;
    pci->device_id = (pci_read_config(pci, 2) >> 16) & 0xffff;
}

static void pci_add_device(pci_device_t *pci) {
    pcis = realloc(pcis, (pcis_len + 1) * sizeof(pci_device_t));
    pcis[pcis_len] = *pci;
    pcis_len++;
}

static void pci_get_bars(pci_device_t *pci) {
    for (int i = 0; i < 6; i++) {
        uint32_t bar = pci_read_config(pci, 0x10 + (i * 4));
        pci->bar[i] = bar & 0xfffffffE;
        pci->bar_is_mem[i] = 0 == (bar & 0x1);
    }
}

static void pci_get_class(pci_device_t *pci) {
    uint16_t upper_word = pci_read_config(pci, 0x0A); // Bits 31-16
    uint16_t lower_word = pci_read_config(pci, 0x08); // Bits 15-0

    uint32_t class_info = ((uint32_t)upper_word << 16) | lower_word; // Combine en un uint32_t

    pci->class_id = (class_info >> 24) & 0xFF;       // Bits 31-2`4

    uint16_t class_code = pci_read_config(pci, 0x08);
    pci->subclass_id = (class_code >> 8) & 0xFF;
    pci->prog_if = class_code & 0xFF;
}

int pci_init(void) {
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
                pci.interrupt_line = pci_read_config(&pci, 0x3C) & 0xFF;
                pci.interrupt_pin = pci_read_config(&pci, 0x3D) & 0xFF;
                pci_add_device(&pci);
            }
        }
    }
    return 0;
}

void pci_write_cmd_u8(pci_device_t *pci, uint8_t barN, uint32_t offset, uint8_t value) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map((void *)(bar_base + offset), (void *)(bar_base + offset), 0);
        volatile uint8_t *mem_addr = (volatile uint8_t *)(bar_base + offset);
        *mem_addr = value;
    } else {
        // Si le BAR est de type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        port_write8(io_port, value); // Assurez-vous que `outb` est implémentée ou disponible
    }
}

void pci_write_cmd_u16(pci_device_t *pci, uint8_t barN, uint32_t offset, uint16_t value) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map((void *)(bar_base + offset), (void *)(bar_base + offset), 0);
        volatile uint16_t *mem_addr = (volatile uint16_t *)(bar_base + offset);
        *mem_addr = value;
    } else {
        // Si le BAR est de type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        port_write16(io_port, value); // Assurez-vous que `outb` est implémentée ou disponible
    }
}

void pci_write_cmd_u32(pci_device_t *pci, uint8_t barN, uint32_t offset, uint32_t value) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map((void *)(bar_base + offset), (void *)(bar_base + offset), 0);
        volatile uint32_t *mem_addr = (volatile uint32_t *)(bar_base + offset);
        *mem_addr = value;
    } else {
        // Si le BAR est de type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        port_write32(io_port, value); // Assurez-vous que `outb` est implémentée ou disponible
    }
}

uint8_t pci_read_cmd_u8(pci_device_t *pci, uint8_t barN, uint32_t offset) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map((void *)(bar_base + offset), (void *)(bar_base + offset), 0);
        return *(volatile uint8_t *)(bar_base + offset);
    } else {
        // Si le BAR est  type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        return port_read8(io_port); // Assurez-vous que `outb` est implémentée ou disponible
    }
}

uint16_t pci_read_cmd_u16(pci_device_t *pci, uint8_t barN, uint32_t offset) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map((void *)(bar_base + offset), (void *)(bar_base + offset), 0);
        return *(volatile uint16_t *)(bar_base + offset);
    } else {
        // Si le BAR est  type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        return port_read16(io_port); // Assurez-vous que `outb` est implémentée ou disponible
    }
}

uint32_t pci_read_cmd_u32(pci_device_t *pci, uint8_t barN, uint32_t offset) {
    uint32_t bar_base = pci->bar[barN];

    if (pci->bar_is_mem[barN]) {
        scuba_call_map((void *)(bar_base + offset), (void *)(bar_base + offset), 0);
        return *(volatile uint32_t *)(bar_base + offset);
    } else {
        // Si le BAR est  type E/S
        uint16_t io_port = (uint16_t)(bar_base + offset);
        return port_read32(io_port); // Assurez-vous que `outb` est implémentée ou disponible
    }
}


pci_device_t *pci_find(uint16_t vendor, uint16_t device) {
    for (int i = 0; i < pcis_len; i++) {
        if (pcis[i].vendor_id == vendor && pcis[i].device_id == device) {
            return pcis + i;
        }
    }
    return NULL;
}

pci_device_t *pci_find_array(pci_findme_t *ids, int count) {
    for (int i = 0; i < pcis_len; i++) {
        for (int j = 0; j < count; j++) {
            if (pcis[i].vendor_id == ids[j].vendor_id && pcis[i].device_id == ids[j].device_id) {
                return pcis + i;
            }
        }
    }
    return NULL;
}

void pci_enable_bus_master(pci_device_t *pci) {
    uint32_t cmd = pci_read_config(pci, 0x04);
    cmd |= (1 << 2);
    pci_write_config(pci, 0x04, cmd);
}


// static volatile uint32_t *lapic = (volatile uint32_t *) LAPIC_DEFAULT_BASE;

void rdmsr(uint32_t msr, uint32_t *value_high, uint32_t *value_low) {
    __asm__ volatile ("rdmsr" : "=d"(*value_high), "=a"(*value_low) : "c"(msr));
}

void wrmsr(uint32_t msr, uint32_t value_low, uint32_t value_high) {
    __asm__ volatile ("wrmsr" : : "a"(value_low), "d"(value_high), "c"(msr));
}

static void lapic_enable() {
    uint32_t apic_base_low;
    uint32_t apic_base_high;
    rdmsr(IA32_APIC_BASE_MSR, &apic_base_high, &apic_base_low);

    apic_base_low |= (1ULL << 11);

    wrmsr(IA32_APIC_BASE_MSR, apic_base_low, apic_base_high);
}

int cpu_has_lapic() {
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(1));
    return (edx & (1 << 9)) != 0;
}

static uint32_t next_int_no = 32 + 28;

uint32_t pci_try_enable_msi(pci_device_t *pci) {
    if (!cpu_has_lapic()) {
        kprintf("RET DEBUG 1\n");
        return 0; // No LAPIC, cannot use MSI
    }
    scuba_call_map((void *) LAPIC_DEFAULT_BASE, (void *) LAPIC_DEFAULT_BASE, 0);
    lapic_enable();
    volatile uint32_t *lapic = (volatile uint32_t *) LAPIC_DEFAULT_BASE;

    uint32_t svr = lapic[0xF0 / 4];
    svr |= 0x100; // APIC Software Enable
    lapic[0xF0 / 4] = svr;

    uint32_t val = pci_read_config(pci, 0x34);
    uint8_t cap_ptr = val & 0xFF;

    while (cap_ptr != 0) {
        uint32_t cap_hdr = pci_read_config(pci, cap_ptr);
        uint8_t cap_id = cap_hdr & 0xFF;
        if (cap_id == PCI_CAP_ID_MSI)
            break;
        cap_ptr = (cap_hdr >> 8) & 0xFF;
    }

    if (cap_ptr == 0) {
        kprintf("No MSI capability\n");
        return 0;
    }

    uint16_t msi_ctrl = pci_read_config_u16(pci, cap_ptr + 2);
    int has_64bit = (msi_ctrl >> 7) & 1;

    uint32_t lapic_addr = LAPIC_DEFAULT_BASE;
    uint32_t data = (next_int_no & 0xFF) | (0 << 8);  // delivery mode = fixed (000), vector = next_int_no

    pci_write_config(pci, cap_ptr + 4, lapic_addr);
    if (has_64bit) {
        pci_write_config(pci, cap_ptr + 8, 0x0);
        pci_write_config(pci, cap_ptr + 12, data);
    } else {
        pci_write_config(pci, cap_ptr + 8, data);
    }

    msi_ctrl |= 0x1; // Enable MSI
    pci_write_config_u16(pci, cap_ptr + 2, msi_ctrl);

    return next_int_no++;
}

