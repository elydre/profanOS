/*****************************************************************************\
|   === e1000.c : 2025 ===                                                    |
|                                                                             |
|    Intel e1000/e network card driver as kernel module            .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/scubasuit.h>
#include <kernel/snowflake.h>
#include <kernel/process.h>
#include <drivers/pci.h>
#include <cpu/ports.h>
#include <cpu/isr.h>
#include <minilib.h>
#include <ktype.h>

#include <modules/eth.h>

//----------------------------------------------------//

#define INTEL_VEND      0x8086      // Vendor ID for Intel
#define E1000_DEV       0x100E      // Device ID for the e1000 Qemu, Bochs, and VirtualBox emmulated NICs
#define E1000_I217      0x153A      // Device ID for Intel I217
#define E1000_82577LM   0x10EA      // Device ID for Intel 82577LM

#define REG_CTRL        0x0000
#define REG_STATUS      0x0008
#define REG_EEPROM      0x0014
#define REG_CTRL_EXT    0x0018
#define REG_IMASK       0x00D0
#define REG_RCTRL       0x0100
#define REG_RXDESCLO    0x2800
#define REG_RXDESCHI    0x2804
#define REG_RXDESCLEN   0x2808
#define REG_RXDESCHEAD  0x2810
#define REG_RXDESCTAIL  0x2818

#define REG_TCTRL       0x0400
#define REG_TXDESCLO    0x3800
#define REG_TXDESCHI    0x3804
#define REG_TXDESCLEN   0x3808
#define REG_TXDESCHEAD  0x3810
#define REG_TXDESCTAIL  0x3818

#define REG_RDTR        0x2820      // RX Delay Timer Register
#define REG_RXDCTL      0x2828      // RX Descriptor Control
#define REG_RADV        0x282C      // RX Int. Absolute Delay Timer
#define REG_RSRPD       0x2C00      // RX Small Packet Detect Interrupt

#define REG_TIPG        0x0410      // Transmit Inter Packet Gap
#define ECTRL_SLU       0x40        // Set link up

#define RCTL_EN             (1 << 1)    // Receiver Enable
#define RCTL_SBP            (1 << 2)    // Store Bad Packets
#define RCTL_UPE            (1 << 3)    // Unicast Promiscuous Enabled
#define RCTL_MPE            (1 << 4)    // Multicast Promiscuous Enabled
#define RCTL_LPE            (1 << 5)    // Long Packet Reception Enable
#define RCTL_LBM_NONE       (0 << 6)    // No Loopback
#define RCTL_LBM_PHY        (3 << 6)    // PHY or external SerDesc loopback
#define RTCL_RDMTS_HALF     (0 << 8)    // Free Buffer Threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER  (1 << 8)    // Free Buffer Threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH   (2 << 8)    // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36          (0 << 12)   // Multicast Offset - bits 47:36
#define RCTL_MO_35          (1 << 12)   // Multicast Offset - bits 46:35
#define RCTL_MO_34          (2 << 12)   // Multicast Offset - bits 45:34
#define RCTL_MO_32          (3 << 12)   // Multicast Offset - bits 43:32
#define RCTL_BAM            (1 << 15)   // Broadcast Accept Mode
#define RCTL_VFE            (1 << 18)   // VLAN Filter Enable
#define RCTL_CFIEN          (1 << 19)   // Canonical Form Indicator Enable
#define RCTL_CFI            (1 << 20)   // Canonical Form Indicator Bit Value
#define RCTL_DPF            (1 << 22)   // Discard Pause Frames
#define RCTL_PMCF           (1 << 23)   // Pass MAC Control Frames
#define RCTL_SECRC          (1 << 26)   // Strip Ethernet CRC

// Buffer Sizes
#define RCTL_BSIZE_256      (3 << 16)
#define RCTL_BSIZE_512      (2 << 16)
#define RCTL_BSIZE_1024     (1 << 16)
#define RCTL_BSIZE_2048     (0 << 16)
#define RCTL_BSIZE_4096     ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192     ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384    ((1 << 16) | (1 << 25))


// Transmit Command
#define CMD_EOP             (1 << 0)    // End of Packet
#define CMD_IFCS            (1 << 1)    // Insert FCS
#define CMD_IC              (1 << 2)    // Insert Checksum
#define CMD_RS              (1 << 3)    // Report Status
#define CMD_RPS             (1 << 4)    // Report Packet Sent
#define CMD_VLE             (1 << 6)    // VLAN Packet Enable
#define CMD_IDE             (1 << 7)    // Interrupt Delay Enable


// TCTL Register
#define TCTL_EN             (1 << 1)    // Transmit Enable
#define TCTL_PSP            (1 << 3)    // Pad Short Packets
#define TCTL_CT_SHIFT       4           // Collision Threshold
#define TCTL_COLD_SHIFT     12          // Collision Distance
#define TCTL_SWXOFF         (1 << 22)   // Software XOFF Transmission
#define TCTL_RTLC           (1 << 24)   // Re-transmit on Late Collision

#define TSTA_DD             (1 << 0)    // Descriptor Done
#define TSTA_EC             (1 << 1)    // Excess Collisions
#define TSTA_LC             (1 << 2)    // Late Collision
#define LSTA_TU             (1 << 3)    // Transmit Underrun

//----------------------------------------------------//

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

struct e1000_rx_desc {
        volatile uint32_t addr_low;
        volatile uint32_t addr_high;
        volatile uint16_t length;
        volatile uint16_t checksum;
        volatile uint8_t status;
        volatile uint8_t errors;
        volatile uint16_t special;
} __attribute__((packed));

struct e1000_tx_desc {
        volatile uint32_t addr_low;
        volatile uint32_t addr_high;
        volatile uint16_t length;
        volatile uint8_t cso;
        volatile uint8_t cmd;
        volatile uint8_t status;
        volatile uint8_t css;
        volatile uint16_t special;
} __attribute__((packed));

typedef struct e1000_rx_desc e1000_rx_desc_t;
typedef struct e1000_tx_desc e1000_tx_desc_t;

typedef struct {
    uint8_t mac[6];
    pci_device_t pci;
    uint8_t irq;
    e1000_rx_desc_t *rx_descs_phys[E1000_NUM_RX_DESC];
    e1000_tx_desc_t *tx_descs_phys[E1000_NUM_TX_DESC];
    uint16_t rx_cur;      // Current Receive Descriptor Buffer
    uint16_t tx_cur;      // Current Transmit Descriptor Buffer
    uint8_t eeprom;
} e1000_t;

static e1000_t g_e1000 = (e1000_t){0};

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

static uint32_t eeprom_read(e1000_t *e1000, uint8_t addr) {
    uint16_t data = 0;
    uint32_t tmp = 0;

    if (e1000->eeprom) {
        pci_write_cmd_u32(&(e1000->pci), 0, REG_EEPROM, 1 | ((uint32_t)(addr) << 8));
        while(!((tmp = pci_read_cmd_u32(&(e1000->pci), 0, REG_EEPROM)) & (1 << 4)));
    }
    else {
        pci_write_cmd_u32(&(e1000->pci), 0, REG_EEPROM, 1 | ((uint32_t)(addr) << 2));
        while(!((tmp = pci_read_cmd_u32(&(e1000->pci), 0, REG_EEPROM)) & (1 << 1)));
    }
    data = (uint16_t)((tmp >> 16) & 0xFFFF);
    return data;
}

void get_mac_address(e1000_t *e1000) {
    if (e1000->eeprom == 0) {
        uint32_t mac_low = pci_read_cmd_u32(&(e1000->pci), 0, 0x5400);
        uint16_t mac_high = pci_read_cmd_u32(&(e1000->pci), 0, 0x5404) & 0xFFFF;
        e1000->mac[0] = mac_low & 0xFF;
        e1000->mac[1] = (mac_low >> 8) & 0xFF;
        e1000->mac[2] = (mac_low >> 16) & 0xFF;
        e1000->mac[3] = (mac_low >> 24) & 0xFF;
        e1000->mac[4] = mac_high & 0xFF;
        e1000->mac[5] = (mac_high >> 8) & 0xFF;
    } else {
        uint32_t temp;
        temp = eeprom_read(e1000, 0);
        e1000->mac[0] = temp &0xff;
        e1000->mac[1] = temp >> 8;
        temp = eeprom_read(e1000, 1);
        e1000->mac[2] = temp &0xff;
        e1000->mac[3] = temp >> 8;
        temp = eeprom_read(e1000, 2);
        e1000->mac[4] = temp &0xff;
        e1000->mac[5] = temp >> 8;
    }
}

int scan_pci_for_e1000(e1000_t *e1000) {
    pci_device_t *pci = pci_find_array((pci_findme_t[]) {
        {INTEL_VEND, E1000_DEV},
        {INTEL_VEND, E1000_I217},
        {INTEL_VEND, E1000_82577LM},
        {INTEL_VEND, 0x15B7}
    }, 3);

    if (pci == NULL)
        return 1;

    e1000->pci = *pci;
    e1000->irq = pci_read_config(pci, 0x3c) & 0xff;

    pci_write_cmd_u32(pci, 0, REG_EEPROM, 0x1);
    for (int i = 0; i < 1000 && !e1000->eeprom; i++) {
        if(pci_read_cmd_u32(pci, 0, REG_EEPROM) & 0x10)
            e1000->eeprom = 1;
        else
            e1000->eeprom = 0;
    }
    get_mac_address(e1000);
    return 0;
}

int e1000_send_packet(const void *p_data, uint16_t p_len) {
    e1000_t *device = &g_e1000;
    device->tx_descs_phys[device->tx_cur]->addr_low = (uint32_t)p_data;
    device->tx_descs_phys[device->tx_cur]->addr_high = 0;
    device->tx_descs_phys[device->tx_cur]->length = p_len;
    device->tx_descs_phys[device->tx_cur]->cmd = CMD_EOP | CMD_IFCS | CMD_RS;
    device->tx_descs_phys[device->tx_cur]->status = 0;
    uint8_t old_cur = device->tx_cur;
    device->tx_cur = (device->tx_cur + 1) % E1000_NUM_TX_DESC;
    pci_write_cmd_u32(&(device->pci), 0, REG_TXDESCTAIL, device->tx_cur);
    while (!(device->tx_descs_phys[old_cur]->status & 0xff))
        process_sleep(process_get_pid(), 1);
    return 0;
}

void e1000_handle_receive(e1000_t *device, registers_t *regs) {
    (void) regs;
    uint16_t old_cur;
    uint8_t got_packet = 0;

    while((device->rx_descs_phys[device->rx_cur]->status & 0x1)) {
        got_packet = 1;
        (void)got_packet;
        uint8_t *buf = (uint8_t *)device->rx_descs_phys[device->rx_cur]->addr_low;
        uint16_t len = device->rx_descs_phys[device->rx_cur]->length;

        eth_recv_packet(buf, len);

        device->rx_descs_phys[device->rx_cur]->status = 0;
        old_cur = device->rx_cur;
        device->rx_cur = (device->rx_cur + 1) % E1000_NUM_RX_DESC;
        pci_write_cmd_u32(&(device->pci), 0, REG_RXDESCTAIL, old_cur);
    }
}

void e1000_handler(registers_t *regs) {
    pci_write_cmd_u32(&(g_e1000.pci), 0, REG_IMASK, 0x1);
    uint32_t status = pci_read_cmd_u32(&(g_e1000.pci), 0, (0xc0));

    if (status) {
        if (status & 0x80)
            e1000_handle_receive(&g_e1000, regs);
        pci_write_cmd_u32(&(g_e1000.pci), 0, (0xc0), status);
    }

    if (status == 0x3) {
    }
}

void e1000_rx_init(e1000_t *e1000) {
    uint8_t *ptr = NULL;
    e1000_rx_desc_t *descs;

    ptr = mem_alloc(sizeof(struct e1000_rx_desc)*E1000_NUM_RX_DESC + 16, 1, 128);

    descs = (struct e1000_rx_desc *) ptr;
    for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
        e1000->rx_descs_phys[i] = (e1000_rx_desc_t *)((uint8_t *)descs + i*16);
        e1000->rx_descs_phys[i]->addr_high = 0;
        e1000->rx_descs_phys[i]->addr_low = (uint32_t)(uint8_t *)(mem_alloc(8192 + 16, 1, 128));
        e1000->rx_descs_phys[i]->status = 0;
    }

    pci_write_cmd_u32(&(e1000->pci), 0,REG_TXDESCLO, 0);
    pci_write_cmd_u32(&(e1000->pci), 0,REG_TXDESCHI, (uint32_t)ptr);

    pci_write_cmd_u32(&(e1000->pci), 0,REG_RXDESCLO, (uint32_t)ptr);
    pci_write_cmd_u32(&(e1000->pci), 0,REG_RXDESCHI, 0);

    pci_write_cmd_u32(&(e1000->pci), 0, REG_RXDESCLEN, E1000_NUM_RX_DESC * 16);

    pci_write_cmd_u32(&(e1000->pci), 0, REG_RXDESCHEAD, 0);
    pci_write_cmd_u32(&(e1000->pci), 0, REG_RXDESCTAIL, E1000_NUM_RX_DESC-1);
    e1000->rx_cur = 0;
    pci_write_cmd_u32(&(e1000->pci), 0, REG_RCTRL, RCTL_EN| RCTL_SBP| RCTL_UPE | RCTL_MPE |
                RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC  | RCTL_BSIZE_8192);
}

void e1000_tx_init(e1000_t *e1000) {
    uint8_t *  ptr;
    struct e1000_tx_desc *descs;

    ptr = (uint8_t *) (mem_alloc(sizeof(struct e1000_tx_desc) * E1000_NUM_TX_DESC, 1, 16));

    descs = (struct e1000_tx_desc *)ptr;
    for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
        e1000->tx_descs_phys[i] = (struct e1000_tx_desc *) ((uint8_t *) descs + i * 16);
        e1000->tx_descs_phys[i]->addr_high = 0;
        e1000->tx_descs_phys[i]->addr_low = 0;
        e1000->tx_descs_phys[i]->cmd = 0;
        e1000->tx_descs_phys[i]->status = TSTA_DD;
    }

    pci_write_cmd_u32(&(e1000->pci), 0, REG_TXDESCHI, 0);
    pci_write_cmd_u32(&(e1000->pci), 0, REG_TXDESCLO, (uint32_t)ptr);

    // now setup total length of descriptors
    pci_write_cmd_u32(&(e1000->pci), 0, REG_TXDESCLEN, E1000_NUM_TX_DESC * 16);

    // setup numbers
    pci_write_cmd_u32(&(e1000->pci), 0, REG_TXDESCHEAD, 0);
    pci_write_cmd_u32(&(e1000->pci), 0, REG_TXDESCTAIL, 0);
    e1000->tx_cur = 0;
    // pci_write_cmd_u32(&(e1000->pci), 0, REG_TCTRL,  TCTL_EN
    //    | TCTL_PSP
    //    | (15 << TCTL_CT_SHIFT)
    //    | (64 << TCTL_COLD_SHIFT)
    //    | TCTL_RTLC);

    // This line of code overrides the one before it but I left both to highlight that the previous one
    // works with e1000 cards, but for the e1000e cards you should set the TCTRL register as follows.
    // For detailed description of each bit, please refer to the Intel Manual. In the case of I217
    // and 82577LM packets will not be sent if the TCTRL is not configured using the following bits.

    pci_write_cmd_u32(&(e1000->pci), 0, REG_TCTRL,  0b0110000000000111111000011111010);
    pci_write_cmd_u32(&(e1000->pci), 0, REG_TIPG,  0x0060200A);
}

static int test();
int __init(void) {
	test();
    if (scan_pci_for_e1000(&g_e1000))
        return 2;

    pci_enable_bus_master(&g_e1000.pci);

    // kprintf("e1000 device found slot %x bus %x\n", g_e1000.pci.slot, g_e1000.pci.bus);
    // kprintf("MAC Address: %x:%x:%x:%x:%x:%x\n",
    //        g_e1000.mac[0], g_e1000.mac[1], g_e1000.mac[2], g_e1000.mac[3], g_e1000.mac[4], g_e1000.mac[5]);

    uint32_t ctrl = pci_read_cmd_u32(&(g_e1000.pci), 0,REG_CTRL);
    pci_write_cmd_u32(&(g_e1000.pci), 0, REG_CTRL, ctrl | (1 << 26));
    while (!(pci_read_cmd_u32(&(g_e1000.pci), 0, REG_STATUS))) {
        // wait the card to be prepared
    }
    for (int i = 0; i < 0x80; i++)
        pci_write_cmd_u32(&(g_e1000.pci), 0, 0x5200 + i * 4, 0);
    interrupt_register_handler(g_e1000.irq + 32, e1000_handler);
    pci_write_cmd_u32(&(g_e1000.pci), 0, REG_IMASK, 0x1F6DC);
    pci_write_cmd_u32(&(g_e1000.pci), 0, REG_IMASK, 0xFF & ~4);
    pci_read_cmd_u32(&(g_e1000.pci), 0, 0xc0);
    e1000_rx_init(&g_e1000);
    e1000_tx_init(&g_e1000);
    pci_write_cmd_u32(&(g_e1000.pci), 0, 0x5400, *((uint32_t *)g_e1000.mac));
    pci_write_cmd_u32(&(g_e1000.pci), 0, 0x5400 + 4, g_e1000.mac[4] | ((uint32_t)g_e1000.mac[5] << 8));

    eth_register_nic(e1000_send_packet, g_e1000.mac);
    return 0;
}

void *__module_func_array[] = {
    (void *) 0xF3A3C4D4, // magic
    // no functions exported
};

#define size_t uint32_t

typedef struct {
	char signature[8];
	uint8_t checksum;
	char OEMID[6];
	uint8_t revision;
	uint32_t rsdt_addr;
} __attribute__ ((packed)) RSDP_t;

typedef struct {
	char signature[8];
	uint8_t checksum;
	char OEMID[6];
	uint8_t revision;
	uint32_t rsdt_addr;
	
	uint32_t len;
	uint64_t xsdt_addr;
	uint8_t extend_checksum;
	uint8_t reserved[3];
} __attribute__ ((packed)) XSDP_t;

typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
} __attribute__ ((packed)) SDT_header_t;

void syscall_scuba_map(void *a, void *b, int c) {
	(void)c;
	scuba_map_func(process_get_dir(process_get_pid()), a, b, 2);
}

void syscall_scuba_unmap(void *a) {
	scuba_unmap(process_get_dir(process_get_pid()), a);
}

#define LAPIC_BASE 0xFEE00000
#define LAPIC_EOI      0x0B0
#define LAPIC_SVR      0x0F0

#define LAPIC_ICR_LOW  0x300
#define LAPIC_ICR_HIGH 0x310

volatile uint32_t *lapic = (uint32_t *)LAPIC_BASE;

void lapic_write(uint32_t reg, uint32_t value) {
	syscall_scuba_map((void *)lapic, (void *)lapic, 0);
    lapic[reg / 4] = value;
	syscall_scuba_unmap((void *)lapic);
}

RSDP_t *find_rsdp() {
	char *start = (void *)0xe0000;
	char *end = (void *)0xFFFFFF;

	while (start < end) {
		if (!mem_cmp(start, "RSD PTR ", 8))
			break;
		start++;
	}

	if (start == end)
		return NULL;
	RSDP_t *res = (void *)start;

	uint8_t sum = 0;
	for (size_t i = 0; i < 20; i++)
		sum += start[i];
	
	if (sum != 0)
		return NULL;
	return res;
}

struct madt_entry {
    uint8_t type;
    uint8_t length;
} __attribute__((packed));


struct madt_lapic {
    uint8_t type;
    uint8_t length;
    uint8_t acpi_cpu_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed));

static inline void io_wait(void) {
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}

static int8_t ap[]  = {
  0xbb, 0x1e, 0x70, 0xba, 0xfd, 0x03, 0xec, 0x24, 0x20, 0x3c, 0x00, 0x74, 0xf6, 0xba, 0xf8, 0x03,
  0x8a, 0x07, 0xee, 0x43, 0x80, 0x3f, 0x00, 0x74, 0x02, 0xeb, 0xe8, 0xf4, 0xeb, 0xfe, 0x73, 0x61,
  0x6c, 0x75, 0x74, 0x20, 0x64, 0x65, 0x70, 0x75, 0x69, 0x73, 0x20, 0x6c, 0x65, 0x20, 0x63, 0x6f,
  0x65, 0x75, 0x72, 0x31, 0x10, 0x00
};

void start_cpu(uint8_t apic_id) {
	mem_copy((void *)0x7000, ap, sizeof(ap));

    lapic_write(LAPIC_ICR_HIGH, ((uint32_t)apic_id) << 24);

    lapic_write(LAPIC_ICR_LOW, 0x00004500);

    io_wait();

    lapic_write(LAPIC_ICR_LOW, 0x00000607);

    io_wait();

    lapic_write(LAPIC_ICR_LOW, 0x00000607);
}

void parse_madt(SDT_header_t *header)
{
    uint8_t *ptr = (uint8_t *)header;

    uint32_t offset = sizeof(SDT_header_t) + 8;

    while(offset < header->length)
    {
        struct madt_entry *entry =
            (void *)(ptr + offset);

        if(entry->type == 0)
        {
            struct madt_lapic *cpu =
                (void *)entry;

            if(cpu->flags & 1)
            {
                kprintf(
                    "CPU ACPI=%d APIC=%d %x\n",
                    cpu->acpi_cpu_id,
                    cpu->apic_id,
					cpu->flags
                );
				if (cpu->acpi_cpu_id == 1)
					start_cpu(cpu->apic_id);
            }
        }

        offset += entry->length;
    }
}

void treat_entry(SDT_header_t *header) {
	if (mem_cmp(header->signature, "APIC", 4))
		return ;
	kprintf("entry %c%c%c%c\n", header->signature[0], header->signature[1], header->signature[2], header->signature[3]);
	parse_madt(header);
}

void foreach_entry(SDT_header_t *header, void (*func)(SDT_header_t *)) {
	syscall_scuba_map(header, header, 0);

	uint8_t sum = 0;
	for (uint32_t i = 0; i < header->length; i++)
		sum += ((uint8_t *)header)[i];

	if (sum != 0) {
		kprintf("Error: checksum invalide for rsdt header\n");
		return ;
	}

	uint32_t entries_len = (header->length - sizeof(SDT_header_t)) / 4;
	kprintf("found %d entries\n", entries_len);

	for (uint32_t i = 0; i < entries_len; i++) {
		uint32_t addr = ((uint32_t)&header[1]) + i * 4;

		SDT_header_t *entry_header = (void *)*(uint32_t *)addr;
		syscall_scuba_map(entry_header, entry_header, 0);
		func(entry_header);
		syscall_scuba_unmap(entry_header);
	}
	syscall_scuba_unmap(header);
}

static int test() {
	RSDP_t *rsdp = find_rsdp();
	if (!rsdp) {
		kprintf("Error: couldn't find rsdp table in (0xe0000-0xfffff)\n");
		return 1;
	}
	kprintf("found rsdp table at %p\n", rsdp);

	if (rsdp->revision < 2) {
		foreach_entry((void *)rsdp->rsdt_addr, treat_entry);
		return 1;
	}
	else {
		XSDP_t *xsdp = (void *)rsdp;
		kprintf("xsdt %x%x\n", (uint32_t)(xsdp->xsdt_addr >> 32), (uint32_t)xsdp->xsdt_addr);
		foreach_entry((void *)(uint32_t)xsdp->xsdt_addr, treat_entry);
	}

	kprintf("\n");
	return 0;
}
