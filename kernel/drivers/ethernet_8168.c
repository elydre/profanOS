#include <cpu/ports.h>
#include <ktype.h>
#include <minilib.h>
#include <drivers/pci.h>
#include <cpu/isr.h>
#include <kernel/snowflake.h>
#include <cpu/timer.h>
#include <net/ethernet.h>

#define NUM_TX_DESC  2
#define NUM_RX_DESC  8
#define TX_BUF_SIZE  0x600
#define RX_BUF_SIZE  0x600

#define OWN 0x80000000
#define EOR 0x40000000
struct Descriptor {
	unsigned int command;
	unsigned int vlan;
	unsigned int low_buf;
	unsigned int high_buf;
};

struct Descriptor *Rx_Descriptors;
struct Descriptor *Tx_Descriptors;
static uint8_t tx_buf_pool[NUM_TX_DESC][TX_BUF_SIZE] __attribute__((aligned(256)));

unsigned long rx_pointer = 0;
unsigned long tx_pointer = 0;

static int this_eth_ids[][2] = {
    {0x10ec, 0x8161},
    {0x10ec, 0x8168},
    {0x10ec, 0x8169},
    {0x1259, 0xc107},
    {0x1737, 0x1032},
    {0x16ec, 0x0116},
    {0, 0}
};

int rtl8168_is_inited = 0;
static uint8_t g_mac_address[6] = {0};
pci_device_t *g_pci_device = NULL;

static volatile uint32_t package_send_ack = 0;

void rtl8168_handler(registers_t *regs) {
    (void)regs;
	kprintf("RTL8168 IRQ\n");
    if (!rtl8168_is_inited) return;


	// Acknowledge the interrupt
	pci_write_cmd_u8(g_pci_device, 0, 0x3C, 0xFF);

	// Read the status register to check for received packets
	// and to clear the interrupt
	unsigned short status = pci_read_cmd_u16(g_pci_device, 0, 0x3E);
	if(status & 0x20) status |= 0x20;
	if(status & 0x01) {
		for(int z = 0; z < NUM_RX_DESC; z++) {
			if (!(Rx_Descriptors[z].command & OWN)) {
                eth_append_packet((void *)Rx_Descriptors[z].low_buf, Rx_Descriptors[z].command & 0x3FFF);
				Rx_Descriptors[z].command |= OWN;
			}
		}
		status |= 0x01;
	}
	if(status & 0x04) {
        package_send_ack = 1;
		status |= 0x04;
	}
	pci_write_cmd_u16(g_pci_device, 0, 0x3E, status);
}

int rtl8168_send_packet(const void *p_data, uint16_t p_len) {
    if (p_len > TX_BUF_SIZE) return;

    volatile struct Descriptor *desz = &Tx_Descriptors[tx_pointer];
    while (desz->command & OWN) {
        kprintf("Waiting for TX descriptor to be free...\n");
    }

    mem_copy(&tx_buf_pool[tx_pointer][0], (void *)p_data, p_len);

    desz->low_buf = (uint32_t)&tx_buf_pool[tx_pointer][0];
    desz->high_buf = 0;
    desz->vlan = 0;
    desz->command = OWN | ((tx_pointer == NUM_TX_DESC - 1) ? EOR : 0) |
                    0x40000 | 0x20000000 | 0x10000000 | (p_len & 0x3FFF);

    package_send_ack = 0;
    pci_write_cmd_u8(g_pci_device, 0, 0x38, 0x40);

    while (1) {
        if (package_send_ack == 1) break;
        if ((pci_read_cmd_u8(g_pci_device, 0, 0x38) & 0x40) == 0) break;
    }
    package_send_ack = 0;
    tx_pointer = (tx_pointer + 1) % NUM_TX_DESC;
    return 0;
}
int rtl8168_init() {
    pci_device_t *pci = NULL;
    for (int i = 0; this_eth_ids[i][0] != 0; i++) {
        pci = pci_find(this_eth_ids[i][0], this_eth_ids[i][1]);
        if (pci != NULL) break;
    }
    if (pci == NULL) return 2;

    pci_enable_bus_master(pci);
    g_pci_device = pci;

    // Activation MSI
    g_pci_device->interrupt_line = pci_try_enable_msi(pci);
    if (g_pci_device->interrupt_line == 0) {
        kprintf("Failed to enable MSI for RTL8168\n");
        return 2;
    }
    interrupt_register_handler(g_pci_device->interrupt_line, rtl8168_handler);

    // Reset matériel de la carte
    pci_write_cmd_u8(pci, 0, 0x52, 0x0);  // Clear some control register (OK)
    pci_write_cmd_u8(pci, 0, 0x37, 0x10); // Software reset bit = 1

    uint32_t start_time = timer_get_ms();
    while (timer_get_ms() - start_time < 500) {
        // Attendre que le reset soit fini (bit 4 à 0)
        if ((pci_read_cmd_u8(pci, 0, 0x37) & 0x10) == 0) break;
    }

    // Lecture adresse MAC
    for (int i = 0; i < 6; i++)
        g_mac_address[i] = pci_read_cmd_u8(pci, 0, i);

    // Allocation des buffers pour RX et TX
    Rx_Descriptors = mem_alloc(sizeof(struct Descriptor)*NUM_RX_DESC, 0xFFFF, 1);
    Tx_Descriptors = mem_alloc(sizeof(struct Descriptor)*NUM_TX_DESC, 0xFFFF, 1);
    mem_set(Rx_Descriptors, 0, sizeof(struct Descriptor)*NUM_RX_DESC);
    mem_set(Tx_Descriptors, 0, sizeof(struct Descriptor)*NUM_TX_DESC);

    for (unsigned long i = 0; i < NUM_RX_DESC; i++) {
        unsigned long rx_buffer_len = RX_BUF_SIZE;
        unsigned long packet_buffer_address = (unsigned long)mem_alloc(rx_buffer_len, 256, 1);
        Rx_Descriptors[i].command = OWN | ((i == (NUM_RX_DESC - 1)) ? EOR : 0) | (rx_buffer_len & 0x3FFF);
        Rx_Descriptors[i].low_buf = (uint32_t)packet_buffer_address;
        Rx_Descriptors[i].high_buf = 0;
    }

    // Tx descriptors
    Tx_Descriptors[NUM_TX_DESC - 1].command |= EOR;

    // Config registre 0x50 : Activer Rx & Tx (bits 6 et 7)
    pci_write_cmd_u8(pci, 0, 0x50, 0xC0);

    // Config filtre Rx, autres paramètres spécifiques à ta carte
    pci_write_cmd_u32(pci, 0, 0x44, 0x0000E70F);
    pci_write_cmd_u8(pci, 0, 0x37, 0x0C); // Activer Rx & Tx (bit 3 et 2)

    pci_write_cmd_u32(pci, 0, 0x40, 0x03000700);
    pci_write_cmd_u16(pci, 0, 0xDA, 0x1FFF);
    pci_write_cmd_u8(pci, 0, 0xEC, 0x3B);

    // Setup adresses des descriptors Tx/Rx dans les registres PCI
    pci_write_cmd_u32(pci, 0, 0x20, (uint32_t)&Tx_Descriptors[0]);
    pci_write_cmd_u32(pci, 0, 0x24, 0);
    pci_write_cmd_u32(pci, 0, 0xE4, (uint32_t)&Rx_Descriptors[0]);
    pci_write_cmd_u32(pci, 0, 0xE8, 0);

    // NE PAS DESACTIVER Rx/Tx à la fin !!!
    // pci_write_cmd_u8(pci, 0, 0x50, 0x00); <--- Supprimé

    rtl8168_is_inited = 1;

    kprintf("MAC ADDR %x:%x:%x:%x:%x:%x\n",
       g_mac_address[0], g_mac_address[1], g_mac_address[2],
       g_mac_address[3], g_mac_address[4], g_mac_address[5]);


    return 0;
}
void rtl8168_set_mac(uint8_t mac[6]) {
    for (int i = 0; i < 6; i++)
        mac[i] = g_mac_address[i];
}