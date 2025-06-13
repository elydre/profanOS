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

typedef struct {
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t status;
    uint32_t length;
} __attribute__((packed)) rtl_desc_t;

static rtl_desc_t *tx_descs;
static rtl_desc_t *rx_descs;
static uint8_t *tx_buffers[NUM_TX_DESC];
static uint8_t *rx_buffers[NUM_RX_DESC];
static int tx_cur = 0;
static int rx_cur = 0;

void init_tx_rx() {
    tx_descs = (rtl_desc_t *)mem_alloc(NUM_TX_DESC * sizeof(rtl_desc_t), 256, 1);
    rx_descs = (rtl_desc_t *)mem_alloc(NUM_RX_DESC * sizeof(rtl_desc_t), 256, 1);

    for (int i = 0; i < NUM_TX_DESC; i++) {
        tx_buffers[i] = (uint8_t *)mem_alloc(TX_BUF_SIZE, 256, 1);
        tx_descs[i].addr_high = 0;
        tx_descs[i].addr_low = (uint32_t)tx_buffers[i]; // Idéalement: vaddr_to_paddr(tx_buffers[i]);
        tx_descs[i].status = 0x80000000;
        tx_descs[i].length = 0;
    }

    for (int i = 0; i < NUM_RX_DESC; i++) {
        rx_buffers[i] = (uint8_t *)mem_alloc(RX_BUF_SIZE, 256, 1);
        rx_descs[i].addr_high = 0;
        rx_descs[i].addr_low = (uint32_t)rx_buffers[i]; // Idéalement: vaddr_to_paddr(rx_buffers[i]);
        rx_descs[i].status = 0x80000000;
        rx_descs[i].length = RX_BUF_SIZE;
    }

    uint32_t io = g_pci_device->bar[0] & ~0x3;

    port_long_out(io + 0x20, (uint32_t)rx_descs); // RX desc addr low
    port_long_out(io + 0x24, 0); // RX desc addr high

    port_long_out(io + 0x10, (uint32_t)tx_descs); // TX desc addr low
    port_long_out(io + 0x14, 0); // TX desc addr high

    port_byte_out(io + 0x2C, 0x6); // RX buffer size = 0x600

    port_byte_out(io + 0x37, 0x0C); // Enable RX & TX

    port_word_out(io + 0x3C, 0x0005); // Enable interrupts
}

void rtl8168_send_packet(const void *p_data, uint16_t p_len) {
    if (!rtl8168_is_inited || p_len > TX_BUF_SIZE)
        return;

    mem_copy(tx_buffers[tx_cur], (void *)p_data, p_len);
    tx_descs[tx_cur].addr_high = 0;
    tx_descs[tx_cur].addr_low = (uint32_t)tx_buffers[tx_cur]; // Idéalement: vaddr_to_paddr
    tx_descs[tx_cur].length = p_len & 0x1FFF;
    tx_descs[tx_cur].status = 0x80000000 | (p_len & 0x1FFF);

    uint32_t io = g_pci_device->bar[0] & ~0x3;
    port_byte_out(io + 0x38, tx_cur); // Notify NIC

    // Timeout pour éviter les blocages
    uint32_t timeout = timer_get_ms() + 100;
    while ((tx_descs[tx_cur].status & 0x80000000) && (timer_get_ms() < timeout))
        ;

    tx_cur = (tx_cur + 1) % NUM_TX_DESC;
}

void rtl8168_handler(registers_t *regs) {
    (void)regs;

    if (!rtl8168_is_inited)
        return;
    kprintf("RTL8168 INT RECV\n");
    uint32_t io = g_pci_device->bar[0] & ~0x3;

    // Traite tous les paquets disponibles
    while (!(rx_descs[rx_cur].status & 0x80000000)) {
        uint32_t length = rx_descs[rx_cur].status & 0x1FFF;

        eth_append_packet(rx_buffers[rx_cur], length);

        rx_descs[rx_cur].status = 0x80000000;
        rx_descs[rx_cur].length = RX_BUF_SIZE;
        rx_cur = (rx_cur + 1) % NUM_RX_DESC;
    }

    // Accuser réception de l'interruption
    port_word_out(io + 0x3E, 0xFFFF);
}

int rtl8168_init() {
    pci_device_t *pci = NULL;
    for (int i = 0; this_eth_ids[i][0] != 0; i++) {
        pci = pci_find(this_eth_ids[i][0], this_eth_ids[i][1]);
        if (pci != NULL)
            break;
    }
    if (pci == NULL)
        return 2;

    pci_enable_bus_master(pci);
    g_pci_device = pci;

    kprintf("Realtek 8168 found at %x:%x:%x\n", pci->vendor_id, pci->device_id, pci->bar[0]);

    // Lire l'adresse MAC
    for (int i = 0; i < 6; i++)
        g_mac_address[i] = port_byte_in(pci->bar[0] + i);

    kprintf("MAC Address: %x:%x:%x:%x:%x:%x\n",
        g_mac_address[0], g_mac_address[1], g_mac_address[2],
        g_mac_address[3], g_mac_address[4], g_mac_address[5]);

    // Reset
    port_byte_out(pci->bar[0] + 0x37, 0x10);
    uint32_t t0 = timer_get_ms();
    while ((port_byte_in(pci->bar[0] + 0x37) & 0x10) && (timer_get_ms() - t0 < 1000))
        ;

    init_tx_rx();

    register_interrupt_handler(pci->interrupt_line + 32, rtl8168_handler);
    port_word_out(pci->bar[0] + 0x3C, 0x0005);

    rtl8168_is_inited = 1;
    return 0;
}

void rtl8168_set_mac(uint8_t mac[6]) {
    for (int i = 0; i < 6; i++)
        mac[i] = g_mac_address[i];
}
