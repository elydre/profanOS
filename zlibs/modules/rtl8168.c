/**
 * @file drivers/net/rtl8169/rtl8169.c
 * @brief RTL1869 network card driver
 * 
 * 
 * @copyright
 * This file is part of the Hexahedron kernel, which is part of the Ethereal Operating System.
 * It is released under the terms of the BSD 3-clause license.
 * Please see the LICENSE file in the main repository for more details.
 * 
 * Copyright (C) 2025 Samuel Stuart
 */

/**
 * @file drivers/net/rtl8169/rtl8169.h
 * @brief RTL8169 network card driver
 * 
 * 
 * @copyright
 * This file is part of the Hexahedron kernel, which is part of the Ethereal Operating System.
 * It is released under the terms of the BSD 3-clause license.
 * Please see the LICENSE file in the main repository for more details.
 * 
 * Copyright (C) 2025 Samuel Stuart
 */

/**** INCLUDES ****/

#include <modules/eth.h>
#include <minilib.h>
#include <cpu/timer.h>
#include <drivers/pci.h>
#include <kernel/snowflake.h>
#include <cpu/isr.h>

#define LOG(A, ...) kprintf(__VA_ARGS__)

/**** DEFINITIONS ****/

/* Registers */
#define RTL8169_REG_IDR0            0x00
#define RTL8169_REG_IDR1            0x01
#define RTL8169_REG_IDR2            0x02
#define RTL8169_REG_IDR3            0x03
#define RTL8169_REG_IDR4            0x04
#define RTL8169_REG_IDR5            0x05  
#define RTL8169_REG_TNPDS           0x20            // Transmit Normal Priority Descriptors
#define RTL8169_REG_CR              0x37            // Command register
#define RTL8169_REG_TPPoll          0x38            // TPPoll
#define RTL8169_REG_IMR             0x3C            // Interrupt mask register
#define RTL8169_REG_ISR             0x3E            // Interrupt status register
#define RTL8169_REG_TCR             0x40            // Transmit control register
#define RTL8169_REG_RCR             0x44            // Receive control register
#define RTL8169_REG_TCTR            0x48            // Timer count register
#define RTL8169_REG_MPC             0x4C            // Missed packet counter (Rx FIFO)
#define RTL8169_REG_9346CR          0x50            // 93C46 command register
#define RTL8169_REG_PHYStatus       0x6C            // PHY (GMII, MII, or TBI) status
#define RTL8169_REG_RMS             0xDA            // Rx packet maximum size
#define RTL8169_REG_RDSAR           0xE4            // Receive descriptor start address
#define RTL8169_REG_MTPS            0xEC            // Max transmit packet size register

/* CR */
#define RTL8169_CR_TE               (1 << 2)        // Transmit enable
#define RTL8169_CR_RE               (1 << 3)        // Receive enable
#define RTL8169_CR_RST              (1 << 4)        // Reset

/* TPPoll */
#define RTL8169_TPPoll_FSWInt       (1 << 0)        // Forced software int
#define RTL8169_TPPoll_NPQ          (1 << 6)        // Normal priority queue polling
#define RTL8169_TPPoll_HPQ          (1 << 7)        // High priority queue polling

/* IMR */
#define RTL8169_IMR_ROK             (1 << 0)        // Rx OK
#define RTL8169_IMR_RER             (1 << 1)        // Rx ERROR
#define RTL8169_IMR_TOK             (1 << 2)        // Tx OK
#define RTL8169_IMR_TER             (1 << 3)        // Tx ERROR
#define RTL8169_IMR_RDU             (1 << 4)        // Rx descriptor unavailable
#define RTL8169_IMR_LINKCHG         (1 << 5)        // Link change
#define RTL8169_IMR_FOVW            (1 << 6)        // Rx FIFO overflow
#define RTL8169_IMR_TDU             (1 << 7)        // Tx descriptor unavailable
#define RTL8169_IMR_SWINT           (1 << 8)        // Software interrupt
#define RTL8169_IMR_TIMEOUT         (1 << 14)       // Timeout interrupt

/* ISR */
#define RTL8169_ISR_ROK             (1 << 0)        // Rx OK
#define RTL8169_ISR_RER             (1 << 1)        // Rx ERROR
#define RTL8169_ISR_TOK             (1 << 2)        // Tx OK
#define RTL8169_ISR_TER             (1 << 3)        // Tx ERROR
#define RTL8169_ISR_RDU             (1 << 4)        // Rx descriptor unavailable
#define RTL8169_ISR_LINKCHG         (1 << 5)        // Link change
#define RTL8169_ISR_FOVW            (1 << 6)        // Rx FIFO overflow
#define RTL8169_ISR_TDU             (1 << 7)        // Tx descriptor unavailable
#define RTL8169_ISR_SWINT           (1 << 8)        // Software interrupt
#define RTL8169_ISR_TIMEOUT         (1 << 14)       // Timeout interrupt
#define RTL8169_ISR_SERR            (1 << 15)       // System error

/* MXDMA sizes for TCR */
#define RTL8169_TCR_MXDMA_SHIFT     8
#define RTL8169_TCR_MXDMA16         (0x00 << RTL8169_TCR_MXDMA_SHIFT)
#define RTL8169_TCR_MXDMA32         (0x01 << RTL8169_TCR_MXDMA_SHIFT)
#define RTL8169_TCR_MXDMA64         (0x02 << RTL8169_TCR_MXDMA_SHIFT)
#define RTL8169_TCR_MXDMA128        (0x03 << RTL8169_TCR_MXDMA_SHIFT)
#define RTL8169_TCR_MXDMA256        (0x04 << RTL8169_TCR_MXDMA_SHIFT)
#define RTL8169_TCR_MXDMA512        (0x05 << RTL8169_TCR_MXDMA_SHIFT)
#define RTL8169_TCR_MXDMA1024       (0x06 << RTL8169_TCR_MXDMA_SHIFT)
#define RTL8169_TCR_MXDMA_UNLIMITED (0x07 << RTL8169_TCR_MXDMA_SHIFT)

/* TCR */
#define RTL8169_TCR_NOCRC           (1 << 16)
#define RTL8169_TCR_LBK_MAC         (1 << 17)

/* MXDMA sizes for RCR */
#define RTL8169_RCR_MXDMA_SHIFT     8
#define RTL8169_RCR_MXDMA16         (0x00 << RTL8169_RCR_MXDMA_SHIFT)
#define RTL8169_RCR_MXDMA32         (0x01 << RTL8169_RCR_MXDMA_SHIFT)
#define RTL8169_RCR_MXDMA64         (0x02 << RTL8169_RCR_MXDMA_SHIFT)
#define RTL8169_RCR_MXDMA128        (0x03 << RTL8169_RCR_MXDMA_SHIFT)
#define RTL8169_RCR_MXDMA256        (0x04 << RTL8169_RCR_MXDMA_SHIFT)
#define RTL8169_RCR_MXDMA512        (0x05 << RTL8169_RCR_MXDMA_SHIFT)
#define RTL8169_RCR_MXDMA1024       (0x06 << RTL8169_RCR_MXDMA_SHIFT)
#define RTL8169_RCR_MXDMA_UNLIMITED (0x07 << RTL8169_RCR_MXDMA_SHIFT)

/* RXFTH sizes for RCR */
#define RTL8169_RCR_RXFTH_SHIFT     13
#define RTL8169_RCR_RXFTH64         (0x02 << RTL8169_RCR_RXFTH_SHIFT)
#define RTL8169_RCR_RXFTH128        (0x03 << RTL8169_RCR_RXFTH_SHIFT)
#define RTL8169_RCR_RXFTH256        (0x04 << RTL8169_RCR_RXFTH_SHIFT)
#define RTL8169_RCR_RXFTH512        (0x05 << RTL8169_RCR_RXFTH_SHIFT)
#define RTL8169_RCR_RXFTH1024       (0x06 << RTL8169_RCR_RXFTH_SHIFT)
#define RTL8169_RCR_RXFTH_UNLIMITED (0x07 << RTL8169_RCR_RXFTH_SHIFT)


/* RCR */
#define RTL8169_RCR_AAP             (1 << 0)        // Accept all packets with destination
#define RTL8169_RCR_APM             (1 << 1)        // Accept physical matches
#define RTL8169_RCR_AM              (1 << 2)        // Accept multicast
#define RTL8169_RCR_AB              (1 << 3)        // Accept broadcast
#define RTL8169_RCR_AR              (1 << 4)        // Accept runt
#define RTL8169_RCR_AER             (1 << 5)        // Acept error
#define RTL8169_RCR_9356SEL         (1 << 6)        // 9346/9356 EEPROM select

/* PHYStatus */
#define RTL8169_PHYStatus_FULLDUP   (1 << 0)        // Full-Duplex status
#define RTL8169_PHYStatus_LINKSTS   (1 << 1)        // Link Ok
#define RTL8169_PHYStatus_10M       (1 << 2)        // 10Mbps link speed
#define RTL8169_PHYStatus_100M      (1 << 3)        // 100Mbps link speed
#define RTL8169_PHYStatus_1000MF    (1 << 4)        // 1000Mbps full-duplex link speed
#define RTL8169_PHYStatus_RXFLOW    (1 << 5)        // Receive flow control
#define RTL8169_PHYStatus_TXFLOW    (1 << 6)        // Transmit flow control

/* Desc command value */
#define RTL8169_DESC_CMD_LGSEN      (1 << 27)       // Large send
#define RTL8169_DESC_CMD_LS         (1 << 28)       // Last segment descriptor
#define RTL8169_DESC_CMD_FS         (1 << 29)       // First segment descriptor
#define RTL8169_DESC_CMD_EOR        (1 << 30)       // End of descriptor
#define RTL8169_DESC_CMD_OWN        (1 << 31)       // Ownership

/* 9346 */
#define RTL8169_9346CR_MODE_CONFIG  (0x3 << 6)

/* Counts */
#define RTL8169_RX_DESC_COUNT       256
#define RTL8169_TX_DESC_COUNT       256
#define RTL8169_RX_BUFFER_SIZE      8192
#define RTL8169_TX_BUFFER_SIZE      8192


/**** TYPES ****/

typedef struct rtl8169_desc {
    uint32_t command;           // Command
    uint32_t vlan;              // VLAN
    uint32_t buffer_lo;         // Low bits of buffer
    uint32_t buffer_hi;         // High bits of buffer
} __attribute__((packed)) rtl8169_desc_t;

typedef struct rtl8169 {
    uint32_t base;             // I/O base address

    uint32_t tx_buffers;       // Tx buffer region
    uint32_t tx_descriptors;   // Tx descriptor regions
    uint32_t tx_current;        // Current Tx index

    uint32_t rx_buffers;       // Rx buffer region
    uint32_t rx_descriptors;   // Rx descriptor regions
    uint32_t rx_current;        // Current Rx index
} rtl8169_t;

rtl8169_t *G_NIC = NULL;


void outportb(unsigned short port, unsigned char data) {
    __asm__ __volatile__("outb %b[Data], %w[Port]" :: [Port] "Nd" (port), [Data] "a" (data));
}

void outportw(unsigned short port, unsigned short data) {
    __asm__ __volatile__("outw %w[Data], %w[Port]" :: [Port] "Nd" (port), [Data] "a" (data));
}

void outportl(unsigned short port, unsigned long data) {
    __asm__ __volatile__("outl %k[Data], %w[Port]" :: [Port] "Nd"(port), [Data] "a" (data));
}

unsigned char inportb(unsigned short port) {
    unsigned char returnValue;
    __asm__ __volatile__("inb %w[Port]" : "=a"(returnValue) : [Port] "Nd" (port));
    return returnValue;
}

unsigned short inportw(unsigned short port) {
    unsigned short returnValue;
    __asm__ __volatile__("inw %w[Port]" : "=a"(returnValue) : [Port] "Nd" (port));
    return returnValue;
}

unsigned long inportl(unsigned short port) {
    unsigned long returnValue;
    __asm__ __volatile__("inl %w[Port]" : "=a"(returnValue) : [Port] "Nd" (port));
    return returnValue;
}

/* Macros */
#define RTL8169_WRITE8(reg, value) outportb(nic->base + reg, (value))
#define RTL8169_WRITE16(reg, value) outportw(nic->base + reg, (value))
#define RTL8169_WRITE32(reg, value) outportl(nic->base + reg, (value))
#define RTL8169_READ8(reg) (inportb(nic->base + reg))
#define RTL8169_READ16(reg) (inportw(nic->base + reg))
#define RTL8169_READ32(reg) (inportl(nic->base + reg))

/**
 * @brief Reset an RTL8169 NIC
 * @param nic The NIC to reset
 * @returns 0 on success, 1 on failure
 */
int rtl8169_reset(rtl8169_t *nic) {
    RTL8169_WRITE8(RTL8169_REG_CR, RTL8169_CR_RST);

    // Timeout
    uint32_t debut = timer_get_ms();
    while (RTL8169_READ8(RTL8169_REG_CR) & RTL8169_CR_RST) {
        if (timer_get_ms() - debut > 1000) {
            LOG("ERR", "Resetting the RTL8169 NIC failed due to timeout\n");
            return 1;
        }
    }

    LOG("INFO", "RTL8169 reset successfully\n");
    return 0;
}

/**
 * @brief Read the MAC address of the NIC
 * @param nic The NIC to read the MAC address of
 * @param mac Destination MAC
 */
int rtl8169_readMAC(rtl8169_t *nic, uint8_t *mac) {
    for (int i = 0; i < 6; i++)
        mac[i] = RTL8169_READ8(RTL8169_REG_IDR0 + i);
    return 0;
}

/**
 * @brief Initialize Rx registers for a NIC
 * @param nic The NIC to initialize Rx registers for
 * @returns 0 on success
 */
int rtl8169_initializeRx(rtl8169_t *nic) {
    // Create regions
    nic->rx_buffers = (uint32_t) mem_alloc(RTL8169_RX_DESC_COUNT * RTL8169_RX_BUFFER_SIZE, 1, 0x1000);
    nic->rx_descriptors = (uint32_t) mem_alloc(RTL8169_RX_DESC_COUNT * sizeof(rtl8169_desc_t), 1, 0x1000);

    LOG("DEBUG", "Rx buffers allocated to %p, descriptors allocated to %p\n", nic->rx_buffers, nic->rx_descriptors);

    // Start building each descriptor
    for (int i = 0; i < RTL8169_RX_DESC_COUNT; i++) {
        rtl8169_desc_t *desc = (rtl8169_desc_t*)(nic->rx_descriptors + (i * sizeof(rtl8169_desc_t)));

        desc->command = RTL8169_RX_BUFFER_SIZE | RTL8169_DESC_CMD_OWN;
        if (i == RTL8169_RX_DESC_COUNT - 1)
            desc->command |= RTL8169_DESC_CMD_EOR;

        // Get buffer
        uint32_t buffer = nic->rx_buffers + (i * RTL8169_RX_BUFFER_SIZE);

        // Setup remaining parameters
        desc->vlan = 0x00000000;
        desc->buffer_lo = buffer;
        desc->buffer_hi = 0;
    }

    // Configure Rx descriptor addresses
    uint32_t desc_phys = nic->rx_descriptors;
    asm volatile("wbinvd");

    RTL8169_WRITE32(RTL8169_REG_RDSAR, desc_phys);
    RTL8169_WRITE32(RTL8169_REG_RDSAR + 4, 0);

    // Enable 1024-byte MXDMA, unlimited RXFTH, accept physica lmatch, broadcast, multicast
    RTL8169_WRITE32(RTL8169_REG_RCR, RTL8169_RCR_MXDMA1024 | RTL8169_RCR_RXFTH_UNLIMITED | RTL8169_RCR_AB | RTL8169_RCR_AM | RTL8169_RCR_APM);

    // Configure MPS
    RTL8169_WRITE16(RTL8169_REG_RMS, 0x1FFF);

    // Initialized OK
    return 0;
}

/**
 * @brief Initialize Tx descriptors for a NIC
 * @param nic The NIC to initialize Tx registers for
 * @returns 0 on success
 */
int rtl8169_initializeTx(rtl8169_t *nic) {    
    // Create regions
    nic->tx_buffers = (uint32_t) mem_alloc(RTL8169_TX_DESC_COUNT * RTL8169_TX_BUFFER_SIZE, 1, 0x1000);
    nic->tx_descriptors = (uint32_t) mem_alloc(RTL8169_TX_DESC_COUNT * sizeof(rtl8169_desc_t), 1, 0x1000);

    LOG("DEBUG", "Tx buffers allocated to %p, descriptors allocated to %p\n", nic->tx_buffers, nic->tx_descriptors);

    // Start building each descriptor
    for (int i = 0; i < RTL8169_TX_DESC_COUNT; i++) {
        rtl8169_desc_t *desc = (rtl8169_desc_t*)(nic->tx_descriptors + (i * sizeof(rtl8169_desc_t)));

        // Get buffer
        uint32_t buffer = nic->tx_buffers + (i * RTL8169_TX_BUFFER_SIZE);

        // Setup parameters
        desc->command = (i == RTL8169_TX_DESC_COUNT - 1) ? RTL8169_DESC_CMD_EOR : 0;
        desc->vlan = 0x00000000;
        desc->buffer_lo = buffer;
        desc->buffer_hi = 0;
    }

    // Configure Tx descriptor addresses
    uint32_t desc_phys = nic->tx_descriptors;
    asm volatile("wbinvd");

    RTL8169_WRITE32(RTL8169_REG_TNPDS, desc_phys);
    RTL8169_WRITE32(RTL8169_REG_TNPDS + 4, 0);

    // I ain't care enough to write the defines for this, enables standard IFG and 1024-byte DMA
    RTL8169_WRITE32(RTL8169_REG_TCR, (0x3 << 24) | (0x6 << 8));

    // Configure MPS
    RTL8169_WRITE16(RTL8169_REG_MTPS, 0x3B);

    return 0;
}

/**
 * @brief RTL8169 receive thread
 * @param context NIC
 */

void rtl8169_recv(void) {
    rtl8169_t *nic = G_NIC;
    
    while (1) {
        // Get descriptor
        rtl8169_desc_t *desc = (rtl8169_desc_t*)(nic->rx_descriptors + (nic->rx_current * sizeof(rtl8169_desc_t)));
        
        // Only descriptors no longer owned are valid
        if (desc->command & RTL8169_DESC_CMD_OWN)
            break;
    
        // Figure out packet length
        uint16_t pkt_length = desc->command & 0x3FFF;

        // Error?
        if (desc->command & (1 << 21)) {
            LOG("ERR", "Error in Rx descriptor\n");
            goto _next_desc;
        }

        // Update NIC statistics
        // nic->n->stats.rx_bytes += pkt_length; PROFANOS
        // nic->n->stats.rx_packets++;

        // Pass it on to the Ethernet handler
        // ethernet_handle((ethernet_packet_t*)(nic->rx_buffers + (nic->rx_current * RTL8169_RX_BUFFER_SIZE)), nic->n, pkt_length);

        kprintf("Received packet of length %d\n", pkt_length);

        eth_recv_packet((void*)(nic->rx_buffers + (nic->rx_current * RTL8169_RX_BUFFER_SIZE)), pkt_length);

    _next_desc:
        nic->rx_current = (nic->rx_current + 1) % RTL8169_RX_DESC_COUNT;
        desc->command |= RTL8169_DESC_CMD_OWN;
    }
}

/**
 * @brief RTL8169 IRQ handler
 * @param context NIC
 */
void rtl8169_irq(registers_t *regs) {
    (void)regs;
    rtl8169_t *nic = G_NIC;

    // Why were we interrupted?
    uint16_t isr = RTL8169_READ16(RTL8169_REG_ISR);

    if (isr == 0)
        return; // Spurious interrupt, ignore

    if (isr & RTL8169_ISR_LINKCHG) {// Update link status
        if (RTL8169_READ8(RTL8169_REG_PHYStatus) & RTL8169_PHYStatus_LINKSTS) {
            kprintf("Link is now UP\n");
        } else {
            kprintf("Link is now DOWN\n");
        }
    }

    // Check for errors
    if (isr & RTL8169_ISR_RER) {
        kprintf("Error in received packet\n");
    }

    if (isr & RTL8169_ISR_TER) {
        kprintf("Error in transmitted packet\n");
    }

    if (isr & RTL8169_ISR_TOK /*&& nic->thr*/) {
        // kprintf("DEBUG: WAWWWWW packet transmitted successfully\n");
    }  

    // Did we get a packet?
    if (isr & RTL8169_ISR_ROK) {
        // kprintf("DEBUG: WAWWWWW new packet received\n");
        rtl8169_recv();
    }

    RTL8169_WRITE16(RTL8169_REG_ISR, 0xFFFF);
    pci_msi_eoi();
}

/**
 * @brief Get link speed as string
 * @param nic The NIC to get the link speed for
 */
char *rtl8169_link(rtl8169_t *nic) {
    if (!(RTL8169_READ8(RTL8169_REG_PHYStatus) & RTL8169_PHYStatus_LINKSTS)) {
        return "DOWN";
    }

    if (RTL8169_READ8(RTL8169_REG_PHYStatus) & RTL8169_PHYStatus_1000MF) {
        return "1000Mbps";
    } else if (RTL8169_READ8(RTL8169_REG_PHYStatus) & RTL8169_PHYStatus_100M) {
        return "100Mbps";
    } else if (RTL8169_READ8(RTL8169_REG_PHYStatus) & RTL8169_PHYStatus_10M) {
        return "10Mbps";
    } else {
        return "???";
    }
} 

/**
 * @brief Send a packet from an RTL8169 NIC
 * @param node Node
 * @param off Offset
 * @param size Size
 * @param buffer Buffer 
 */
int rtl8169_send(const void *buffer, uint16_t size) {
    if (!size)
        return 0;

    rtl8169_t *nic = G_NIC;

    // Get current descriptor
    rtl8169_desc_t *desc = (rtl8169_desc_t*)(nic->tx_descriptors + (nic->tx_current * sizeof(rtl8169_desc_t)));

    // Is the descriptor busy?
    if (desc->command & RTL8169_DESC_CMD_OWN) {
        LOG("ERR", "No free Tx descriptors available, cannot send packet\n");
        return 1;
    }

    if (desc->buffer_lo != (nic->tx_buffers + (nic->tx_current * RTL8169_TX_BUFFER_SIZE))) {
        LOG("ERR", "Tx descriptor buffer address mismatch, cannot send packet\n");
        return 1;
    }

    // The descriptor is ready, let's go
    mem_copy((void *) desc->buffer_lo, buffer, size);

    int was_eor = desc->command & RTL8169_DESC_CMD_EOR;

    // Give ownership
    desc->command = size | RTL8169_DESC_CMD_OWN | RTL8169_DESC_CMD_LS | RTL8169_DESC_CMD_FS | (was_eor ? RTL8169_DESC_CMD_EOR : 0);
    desc->vlan = 0;

    // Advance tx_current
    nic->tx_current = (nic->tx_current + 1) % RTL8169_TX_DESC_COUNT;

    // Inform NIC gracefully
    RTL8169_WRITE8(RTL8169_REG_TPPoll, RTL8169_TPPoll_NPQ);

    kprintf("Sent packet of length %d\n", size);

    // nnic->stats.tx_bytes += size;
    // nnic->stats.tx_packets++;

    return size;
}

pci_findme_t this_eth_ids[] = {
    {0x10ec, 0x8161},
    {0x10ec, 0x8168},
    {0x10ec, 0x8169},
    {0x1259, 0xc107},
    {0x1737, 0x1032},
    {0x16ec, 0x0116},
};

/**
 * @brief Initialize a RTL8169 NIC
 * @param device The PCI device
 */
int __init(void) {
    pci_device_t *device = pci_find_array(this_eth_ids, sizeof(this_eth_ids) / sizeof(pci_findme_t));

    if (device == NULL)
        return 2;

    if (sizeof(rtl8169_desc_t) != 16) {
        LOG("ERR", "rtl8169_desc_t is not 16 bytes, cannot continue\n");
        return 1;
    }

    LOG("DEBUG", "A Initializing a RTL8169 NIC (bus %d slot %d func %d)\n", device->bus, device->slot, device->function);
    
    // Get BAR
    uint32_t bar = device->bar[0];

    if (device->bar_is_mem[0]) {
        LOG("ERR", "only I/O BARs are supported for RTL8169\n\n");
        return 1;
    }

    // Create RTL8169 NIC object
    rtl8169_t *nic = calloc(sizeof(rtl8169_t));
    G_NIC = nic;

    nic->base = bar & 0xFFFFFFF0;

    kprintf("I/O base address: %x\n", nic->base);

    // Reset the NIC
    if (rtl8169_reset(nic)) {
        LOG("ERR", "Error while initializing RTL8169\n");
        free(nic);
        return 1;
    }

    // Get the MAC address of the NIC
    uint8_t mac[6];
    if (rtl8169_readMAC(nic, mac)) {
        LOG("ERR", "Error while initializing RTL8169\n");
        free(nic);
        return 1;
    }

    LOG("DEBUG", "MAC: %x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    // Register IRQ handler
    uint32_t irq = pci_try_enable_msi(device);
    if (irq == 0) {
        LOG("ERR", "Failed to enable MSI for RTL8169\n");
        return 1;
    }
    
    interrupt_register_handler(irq, rtl8169_irq);

    LOG("DEBUG", "Registered IRQ%d for NIC\n", irq);

    // Enable configuration registers
    RTL8169_WRITE8(RTL8169_REG_9346CR, RTL8169_9346CR_MODE_CONFIG);

    // Initialize Rx
    if (rtl8169_initializeRx(nic)) {
        LOG("ERR", "Error while initializing RTL8169\n");
        free(nic);
        return 1;
    }

    // Initialize Tx
    if (rtl8169_initializeTx(nic)) {
        LOG("ERR", "Error while initializing RTL8169\n");
        free(nic);
        return 1;
    }

    // Enable receive and transmit
    RTL8169_WRITE8(RTL8169_REG_CR, RTL8169_CR_RE | RTL8169_CR_TE);

    RTL8169_WRITE16(RTL8169_REG_ISR, 0xFFFF); // this is what sends me the ISR with 0
    RTL8169_WRITE16(RTL8169_REG_IMR, 0xFFFF);

    // Enable interrupts
    /* RTL8169_WRITE16(RTL8169_REG_IMR, RTL8169_IMR_ROK | RTL8169_IMR_RER | 
                RTL8169_IMR_TOK | RTL8169_IMR_TER | RTL8169_IMR_RDU |
                RTL8169_IMR_LINKCHG | RTL8169_IMR_FOVW | RTL8169_IMR_TDU); */

    // Update link status
    kprintf("Link status: %s\n", rtl8169_link(nic));

    eth_register_nic(rtl8169_send, mac);

    return 0;
}

void *__module_func_array[] = {
    (void *) 0xF3A3C4D4 // magic
};
