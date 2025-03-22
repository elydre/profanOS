/*****************************************************************************\
|   === e1000.c : 2025 ===                                                    |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// #include <cpu/ports.h>
// #include <ktype.h>
// #include <minilib.h>
// #include <drivers/pci.h>
// #include <cpu/isr.h>
// #include <drivers/e1000.h>


// //----------------------------------------------------//


// #define E1000_NUM_RX_DESC 32
// #define E1000_NUM_TX_DESC 8

// #define TSTA_DD                         (1 << 0)    // Descriptor Done
// #define TSTA_EC                         (1 << 1)    // Excess Collisions
// #define TSTA_LC                         (1 << 2)    // Late Collision
// #define LSTA_TU                         (1 << 3)    // Transmit Underrun

// void hexdump(const unsigned char *tab, int len) {
//     for (int i = 0; i < len; i++) {
//         unsigned char byte = tab[i];
//         kprintf("%c%c", "0123456789ABCDEF"[byte >> 4], "0123456789ABCDEF"[byte & 0x0f]);
//         // Espacement entre les octets
//         if ((i + 1) % 16 == 0 || i == len - 1) {
//             kprintf("\n"); // Saut de ligne toutes les 16 valeurs ou à la fin
//         } else {
//             kprintf(" ");
//         }
//     }
// }



// struct e1000_rx_desc {
//         volatile uint32_t addr_low;
//         volatile uint32_t addr_high;
//         volatile uint16_t length;
//         volatile uint16_t checksum;
//         volatile uint8_t status;
//         volatile uint8_t errors;
//         volatile uint16_t special;
// } __attribute__((packed));

// struct e1000_tx_desc {
//         volatile uint32_t addr_low;
//         volatile uint32_t addr_high;
//         volatile uint16_t length;
//         volatile uint8_t cso;
//         volatile uint8_t cmd;
//         volatile uint8_t status;
//         volatile uint8_t css;
//         volatile uint16_t special;
// } __attribute__((packed));

// typedef struct e1000_rx_desc e1000_rx_desc_t;
// typedef struct e1000_tx_desc e1000_tx_desc_t;

// typedef struct {
//     uint8_t exists;
//     uint8_t mac[6];
//     pci_device_t pci;
//     uint8_t irq;
//     e1000_rx_desc_t *rx_descs_phys[E1000_NUM_RX_DESC];
//     e1000_tx_desc_t *tx_descs_phys[E1000_NUM_TX_DESC];
//     uint16_t rx_cur;      // Current Receive Descriptor Buffer
//     uint16_t tx_cur;      // Current Transmit Descriptor Buffer
// } e1000_t;

// uint32_t endian_switch_u32(uint32_t x) {
//     uint32_t res = 0;
//     res |= (x & 0xff) << 24 ;
//     res |= ((x >> 8) & 0xff) << 16;
//     res |= ((x >> 16) & 0xff) << 8;
//     res |= ((x >> 24) & 0xff);
//     return res;
// }

// uint16_t endian_switch_u16(uint16_t x) {
//     return ((x & 0xff) << 8) | ((x >> 8) & 0xff);
// }

// void get_mac_address(pci_device_t *pci, uint8_t mac[6]) {
//     // addr mac (e1000) at 0x5400
//     uint32_t mac_low = pci_read_u32(pci, 0, 0x5400);
//     uint16_t mac_high = pci_read_u32(pci, 0, 0x5404) & 0xFFFF;

//     mac[0] = mac_low & 0xFF;
//     mac[1] = (mac_low >> 8) & 0xFF;
//     mac[2] = (mac_low >> 16) & 0xFF;
//     mac[3] = (mac_low >> 24) & 0xFF;
//     mac[4] = mac_high & 0xFF;
//     mac[5] = (mac_high >> 8) & 0xFF;
// }

// void scan_pci_for_e1000(e1000_t *e1000) {
//     pci_device_t pci;
//     int found  = 0;
//     //while (1) {
//     //    int is_last = get_next_pci(&pci, 0);
//     //    if (pci.exists && pci.vendor_id == 0x8086 && pci.device_id == 0x100e) {
//     //        found = 1;
//     //        e1000->pci = pci;
//     //        break;
//     //    }
//     //    if (is_last)
//     //        break;
//     //}
//     if (found) {
//         get_mac_address(&(e1000->pci), e1000->mac);
//         e1000->pci = pci;
//         e1000->exists = 1;
//         e1000->irq = pci_config_read(pci.bus, pci.slot, 0, 0x3c) & 0xff;
//         return ;
//     }
// }

// e1000_t e1000_device = {0};

// void e1000_handler(registers_t *regs) {
//     pci_write_u32(&(e1000_device.pci), 0, REG_IMASK, 0x1);
//     uint32_t status = pci_read_u32(&(e1000_device.pci), 0, (0xc0));
//     kprintf("E1000 interrupt %x\n", status);
//     if(status) {
//         if (status & 0x80)
//             e1000_handle_receive(&e1000_device, regs);
//         pci_write_u32(&(e1000_device.pci), 0, (0xc0), status);
//         if (regs->int_no > 0x20 + 8)
//             port_byte_out(0xa0, 0x20);
//         port_byte_out(0x20, 0x20);
//     }
// }

// void e1000_rx_init(e1000_t *e1000) {
//     uint8_t *ptr = NULL;
//     e1000_rx_desc_t *descs;

//     ptr = malloc(sizeof(struct e1000_rx_desc)*E1000_NUM_RX_DESC + 16);

//     descs = (struct e1000_rx_desc *)ptr;
//     for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
//         e1000->rx_descs_phys[i] = (e1000_rx_desc_t *)((uint8_t *)descs + i*16);
//         e1000->rx_descs_phys[i]->addr_high = 0;
//         e1000->rx_descs_phys[i]->addr_low = (uint32_t)(uint8_t *)(malloc(8192 + 16));
//         e1000->rx_descs_phys[i]->status = 0;
//     }
//     pci_write_u32(&(e1000->pci), 0,REG_TXDESCLO, 0);
//     pci_write_u32(&(e1000->pci), 0,REG_TXDESCHI, (uint32_t)ptr);


//     pci_write_u32(&(e1000->pci), 0,REG_RXDESCLO, (uint32_t)ptr);
//     pci_write_u32(&(e1000->pci), 0,REG_RXDESCHI, 0);

//     pci_write_u32(&(e1000->pci), 0, REG_RXDESCLEN, E1000_NUM_RX_DESC * 16);

//     pci_write_u32(&(e1000->pci), 0, REG_RXDESCHEAD, 0);
//     pci_write_u32(&(e1000->pci), 0, REG_RXDESCTAIL, E1000_NUM_RX_DESC-1);
//     e1000->rx_cur = 0;
//     pci_write_u32(&(e1000->pci), 0, REG_RCTRL, RCTL_EN| RCTL_SBP| RCTL_UPE | RCTL_MPE | RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC  | RCTL_BSIZE_8192);
// }

// void e1000_tx_init(e1000_t *e1000) {
//     uint8_t *  ptr;
//     struct e1000_tx_desc *descs;
//     // Allocate buffer for receive descriptors. For simplicity, in my case khmalloc returns a virtual address that is identical to it physical mapped address.
//     // In your case you should handle virtual and physical addresses as the addresses passed to the NIC should be physical ones
//     ptr = (uint8_t *)(malloc(sizeof(struct e1000_tx_desc)*E1000_NUM_TX_DESC + 16));

//     descs = (struct e1000_tx_desc *)ptr;
//     for(int i = 0; i < E1000_NUM_TX_DESC; i++)
//     {
//         e1000->tx_descs_phys[i] = (struct e1000_tx_desc *)((uint8_t*)descs + i*16);
//         e1000->tx_descs_phys[i]->addr_high = 0;
//         e1000->tx_descs_phys[i]->addr_low = 0;
//         e1000->tx_descs_phys[i]->cmd = 0;
//         e1000->tx_descs_phys[i]->status = TSTA_DD;
//     }

//     pci_write_u32(&(e1000->pci), 0, REG_TXDESCHI, (uint32_t)((uint64_t)ptr >> 32) );
//     pci_write_u32(&(e1000->pci), 0, REG_TXDESCLO, (uint32_t)((uint64_t)ptr & 0xFFFFFFFF));


//     //now setup total length of descriptors
//     pci_write_u32(&(e1000->pci), 0, REG_TXDESCLEN, E1000_NUM_TX_DESC * 16);


//     //setup numbers
//     pci_write_u32(&(e1000->pci), 0, REG_TXDESCHEAD, 0);
//     pci_write_u32(&(e1000->pci), 0, REG_TXDESCTAIL, 0);
//     e1000->tx_cur = 0;
//     pci_write_u32(&(e1000->pci), 0, REG_TCTRL,  TCTL_EN
//         | TCTL_PSP
//         | (15 << TCTL_CT_SHIFT)
//         | (64 << TCTL_COLD_SHIFT)
//         | TCTL_RTLC);

//     // This line of code overrides the one before it but I left both to highlight that the previous one works with e1000 cards, but for the e1000e cards
//     // you should set the TCTRL register as follows. For detailed description of each bit, please refer to the Intel Manual.
//     // In the case of I217 and 82577LM packets will not be sent if the TCTRL is not configured using the following bits.
//     pci_write_u32(&(e1000->pci), 0, REG_TCTRL,  0b0110000000000111111000011111010);
//     pci_write_u32(&(e1000->pci), 0, REG_TIPG,  0x0060200A);
// }

// int e1000_init(void) {
//     e1000_t device = {0};
//     scan_pci_for_e1000(&device);
//     e1000_device = device;
//     if (!device.exists) {
//         kprintf("Error: e1000 device noot found\n");
//         return 1;
//     }
//     kprintf("e1000 device found slot %x bus %x\n", device.pci.slot, device.pci.bus);
//     kprintf("MAC Address: %x:%x:%x:%x:%x:%x\n",
//            device.mac[0], device.mac[1], device.mac[2], device.mac[3], device.mac[4], device.mac[5]);

//     uint32_t ctrl = pci_read_u32(&(device.pci), 0,REG_CTRL);
//     pci_write_u32(&(device.pci), 0, REG_CTRL, ctrl | (1 << 26));
//     while (!(pci_read_u32(&(device.pci), 0, REG_STATUS))) {
//         // wait the card to be prepared
//     }
//     for (int i = 0; i < 0x80; i++)
//         pci_write_u32(&(device.pci), 0, 0x5200 + i * 4, 0);
//     asm volatile ("cli");
//     register_interrupt_handler(device.irq + 32, e1000_handler);
//     pci_write_u32(&(device.pci), 0, REG_IMASK, 0x1F6DC);
//     pci_write_u32(&(device.pci), 0, REG_IMASK, 0xFF & ~4);
//     pci_read_u32(&(device.pci), 0, 0xc0);
//     e1000_rx_init(&device);
//     e1000_tx_init(&device);
//     asm volatile ("sti");
//     pci_write_u32(&(device.pci), 0, 0x5400, *((uint32_t *)device.mac));
//     pci_write_u32(&(device.pci), 0, 0x5400 + 4, device.mac[4] | ((uint32_t)device.mac[5] << 8));

//     //get gooogle mac address
//     send_arp(&device, (uint8_t [4]){8, 8, 8, 8});
//     return 0;
// }


// int e1000_sendPacket(e1000_t *device, const void * p_data, uint16_t p_len)
// {
//     device->tx_descs_phys[device->tx_cur]->addr_low = (uint32_t)p_data;
//     device->tx_descs_phys[device->tx_cur]->length = p_len;
//     device->tx_descs_phys[device->tx_cur]->cmd = CMD_EOP | CMD_IFCS | CMD_RS;
//     device->tx_descs_phys[device->tx_cur]->status = 0;
//     uint8_t old_cur = device->tx_cur;
//     device->tx_cur = (device->tx_cur + 1) % E1000_NUM_TX_DESC;
//     pci_write_u32(&(device->pci), 0, REG_TXDESCTAIL, device->tx_cur);
//     kprintf("STRRRRRRRRRRR1\n");
//     while(!(device->tx_descs_phys[old_cur]->status & 0xff));
//     kprintf("STRRRRRRRRRRR2\n");
//     while (1);
//     return 0;
// }


// void e1000_handle_receive(e1000_t *device, registers_t *regs) {

//     uint16_t old_cur;
//     uint8_t got_packet = 0;

//     while((device->rx_descs_phys[device->rx_cur]->status & 0x1))
//     {
//             got_packet = 1;
//             uint8_t *buf = (uint8_t *)device->rx_descs_phys[device->rx_cur]->addr_low;
//             uint16_t len = device->rx_descs_phys[device->rx_cur]->length;

//             kprintf("pack len: %d\n", len);
//             for (int i = 0; i < len; i++)
//                 kprintf("pack: %x\n", buf[i]);


//             device->rx_descs_phys[device->rx_cur]->status = 0;
//             old_cur = device->rx_cur;
//             device->rx_cur = (device->rx_cur + 1) % E1000_NUM_RX_DESC;
//             pci_write_u32(&(device->pci), 0, REG_RXDESCTAIL, old_cur);
//     }
// }

// typedef struct {
//     uint8_t htype;
//     uint8_t ptype;
//     uint16_t hlen;
//     uint16_t plen;
//     uint16_t oper;
//     uint8_t sha[6];
//     uint8_t spa[4];
//     uint8_t tha[6];
//     uint8_t tpa[4];
// } arp_packet;

// void send_arp(e1000_t *device, uint8_t *ip) {
//     arp_packet *packet = malloc(sizeof(arp_packet));
//     packet->htype = 1;
//     packet->ptype = 0x0800;
//     packet->hlen = 6;
//     packet->plen = 4;
//     packet->oper = 1;
//     packet->sha[0] = device->mac[0];
//     packet->sha[1] = device->mac[1];
//     packet->sha[2] = device->mac[2];
//     packet->sha[3] = device->mac[3];
//     packet->sha[4] = device->mac[4];
//     packet->sha[5] = device->mac[5];
//     packet->spa[0] = ip[0];
//     packet->spa[1] = ip[1];
//     packet->spa[2] = ip[2];
//     packet->spa[3] = ip[3];
//     packet->tha[0] = 0x00;
//     packet->tha[1] = 0x00;
//     packet->tha[2] = 0x00;
//     packet->tha[3] = 0x00;
//     packet->tha[4] = 0x00;
//     packet->tha[5] = 0x00;
//     packet->tpa[0] = 0x00;
//     packet->tpa[1] = 0x00;
//     packet->tpa[2] = 0x00;
//     packet->tpa[3] = 0x00;
//     e1000_sendPacket(device, packet, sizeof(arp_packet));
//
