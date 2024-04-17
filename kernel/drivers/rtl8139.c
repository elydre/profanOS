#include <drivers/pci.h>
#include <cpu/ports.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>
#include <ktype.h>
#include <cpu/ports.h>

// Define some constants
#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

#define RX_BUF_SIZE 8192

#define CAPR 0x38
#define RX_READ_POINTER_MASK (~3)
#define ROK                 (1<<0)
#define RER                 (1<<1)
#define TOK     (1<<2)
#define TER     (1<<3)
#define TX_TOK  (1<<15)

enum RTL8139_registers {
  MAG0             = 0x00,       // Ethernet hardware address
  MAR0             = 0x08,       // Multicast filter
  TxStatus0        = 0x10,       // Transmit status (Four 32bit registers)
  TxAddr0          = 0x20,       // Tx descriptors (also four 32bit)
  RxBuf            = 0x30,
  RxEarlyCnt       = 0x34,
  RxEarlyStatus    = 0x36,
  ChipCmd          = 0x37,
  RxBufPtr         = 0x38,
  RxBufAddr        = 0x3A,
  IntrMask         = 0x3C,
  IntrStatus       = 0x3E,
  TxConfig         = 0x40,
  RxConfig         = 0x44,
  Timer            = 0x48,        // A general-purpose counter
  RxMissed         = 0x4C,        // 24 bits valid, write clears
  Cfg9346          = 0x50,
  Config0          = 0x51,
  Config1          = 0x52,
  FlashReg         = 0x54,
  GPPinData        = 0x58,
  GPPinDir         = 0x59,
  MII_SMI          = 0x5A,
  HltClk           = 0x5B,
  MultiIntr        = 0x5C,
  TxSummary        = 0x60,
  MII_BMCR         = 0x62,
  MII_BMSR         = 0x64,
  NWayAdvert       = 0x66,
  NWayLPAR         = 0x68,
  NWayExpansion    = 0x6A,

  // Undocumented registers, but required for proper operation
  FIFOTMS          = 0x70,        // FIFO Control and test
  CSCR             = 0x74,        // Chip Status and Configuration Register
  PARA78           = 0x78,
  PARA7c           = 0x7c,        // Magic transceiver parameter register
};

typedef struct tx_desc {
    uint32_t phys_addr;
    uint32_t packet_size;
}tx_desc_t;

typedef struct rtl8139_dev {
    uint8_t bar_type;
    uint16_t io_base;
    uint32_t mem_base;
    int eeprom_exist;
    uint8_t mac_addr[6];
    char rx_buffer[8192 + 16 + 1500];
    int tx_cur;
}rtl8139_dev_t;

pci_dev_t pci_rtl8139_device;
rtl8139_dev_t rtl8139_device;

uint32_t current_packet_ptr;

// Four TXAD register, you must use a different one to send packet each time(for example, use the first one, second... fourth and back to the first)
uint8_t TSAD_array[4] = {0x20, 0x24, 0x28, 0x2C};
uint8_t TSD_array[4] = {0x10, 0x14, 0x18, 0x1C};

void receive_packet() {
    uint16_t * t = (uint16_t*)(rtl8139_device.rx_buffer + current_packet_ptr);
    // Skip packet header, get packet length
    uint16_t packet_length = *(t + 1);

    // Skip, packet header and packet length, now t points to the packet data
    t = t + 2;

    // Now, ethernet layer starts to handle the packet(be sure to make a copy of the packet, insteading of using the buffer)
    void * packet = malloc(packet_length);
    // 
    mem_move(packet, t, packet_length);
    //ethernet_handle_packet(packet, packet_length);

    current_packet_ptr = (current_packet_ptr + packet_length + 4 + 3) & RX_READ_POINTER_MASK;

    if(current_packet_ptr > RX_BUF_SIZE)
        current_packet_ptr -= RX_BUF_SIZE;

    outports(rtl8139_device.io_base + CAPR, current_packet_ptr - 0x10);
}

void rtl8139_handler(registers_t* reg) {
    if (DEBUG) {kprintf("RTL8139 interript was fired !!!! \n");}
    uint16_t status = inports(rtl8139_device.io_base + 0x3e);

    if(status & TOK) {
        if (DEBUG) {kprintf("Packet sent\n");}
    }
    if (status & ROK) {
        if (DEBUG) {kprintf("Received packet\n");}
        receive_packet();
    }

    outports(rtl8139_device.io_base + 0x3E, 0x5);
}

int rtl8139_init() {
    // First get the network device using PCI
    pci_rtl8139_device = pci_get_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID, -1);
    uint32_t ret = pci_read(pci_rtl8139_device, PCI_BAR0);
    rtl8139_device.bar_type = ret & 0x1;

    // Get io base or mem base by extracting the high 28/30 bits
    rtl8139_device.io_base = ret & (~0x3);
    rtl8139_device.mem_base = ret & (~0xf);
    if (DEBUG) {kprintf("rtl8139 use %s access (base: %x)\n", (rtl8139_device.bar_type == 0)? "mem based":"port based", (rtl8139_device.bar_type != 0)?rtl8139_device.io_base:rtl8139_device.mem_base);}

    // Set current TSAD
    rtl8139_device.tx_cur = 0;

    // Enable PCI Bus Mastering
    uint32_t pci_command_reg = pci_read(pci_rtl8139_device, PCI_COMMAND);
    if(!(pci_command_reg & (1 << 2))) {
        pci_command_reg |= (1 << 2);
        pci_write(pci_rtl8139_device, PCI_COMMAND, pci_command_reg);
    }
    if (DEBUG) {kprintf("Bus mastering enabled\n");}

    // Send 0x00 to the CONFIG_1 register (0x52) to set the LWAKE + LWPTN to active high. this should essentially *power on* the device.
    outportb(rtl8139_device.io_base + 0x52, 0x0);

    // Soft reset
    outportb(rtl8139_device.io_base + 0x37, 0x10);
    while((inportb(rtl8139_device.io_base + 0x37) & 0x10) != 0) {
        // we shall wait for the rest of eternity (or not, who knows)
    }

    // Allocate receive buffer
    outportl(rtl8139_device.io_base + 0x30, (uint32_t) rtl8139_device.rx_buffer);

    // Sets the TOK and ROK bits high
    outports(rtl8139_device.io_base + 0x3C, 0x0005);

    // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP
    outportl(rtl8139_device.io_base + 0x44, 0xf | (1 << 7));

    // Sets the RE and TE bits high
    outportb(rtl8139_device.io_base + 0x37, 0x0C);

    // Register and enable network interrupts
    uint32_t irq_num = pci_read(pci_rtl8139_device, PCI_INTERRUPT_LINE);
    register_interrupt_handler(32 + irq_num, rtl8139_handler);

    read_mac_addr();

    return 0;
}

// NOW WORKING ! letsgo
// maybe i should store it in a global variable?
void read_mac_addr() {
    uint32_t mac_part1 = inportl(rtl8139_device.io_base + 0x00);
    uint16_t mac_part2 = inports(rtl8139_device.io_base + 0x04);
    rtl8139_device.mac_addr[0] = mac_part1 >> 0;
    rtl8139_device.mac_addr[1] = mac_part1 >> 8;
    rtl8139_device.mac_addr[2] = mac_part1 >> 16;
    rtl8139_device.mac_addr[3] = mac_part1 >> 24;

    rtl8139_device.mac_addr[4] = mac_part2 >> 0;
    rtl8139_device.mac_addr[5] = mac_part2 >> 8;

    kprintf("MAC Address: %x:%x:%x:%x:%x:%x\n", rtl8139_device.mac_addr[0], rtl8139_device.mac_addr[1], rtl8139_device.mac_addr[2], rtl8139_device.mac_addr[3], rtl8139_device.mac_addr[4], rtl8139_device.mac_addr[5]);
}


void rtl8139_call(int thing) {
    switch(thing) {
        case 0:
            read_mac_addr();
            break;
        default:
            return;
    }
}
