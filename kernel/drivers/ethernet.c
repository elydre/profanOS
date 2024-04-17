#include <cpu/ports.h>
#include <gui/vesa.h>
#include <cpu/isr.h>
#include <minilib.h>
#include <system.h>
#include <ktype.h>

#define DEBUG 0

void outportb(uint16_t port, uint8_t val) {
    asm volatile("outb %1, %0" : : "dN"(port), "a"(val));
}

uint8_t inportb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint16_t inports(uint16_t _port) {
    uint16_t rv;
    asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outports(uint16_t _port, uint16_t _data) {
    asm volatile ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

uint32_t inportl(uint16_t _port) {
    uint32_t rv;
    asm volatile ("inl %%dx, %%eax" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportl(uint16_t _port, uint32_t _data) {
    asm volatile ("outl %%eax, %%dx" : : "dN" (_port), "a" (_data));
}

/* PCI */

typedef union pci_dev {
    uint32_t bits;
    struct {
        uint32_t always_zero    : 2;
        uint32_t field_num      : 6;
        uint32_t function_num   : 3;
        uint32_t device_num     : 5;
        uint32_t bus_num        : 8;
        uint32_t reserved       : 7;
        uint32_t enable         : 1;
    };
} pci_dev_t;

// Ports
#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

// Offset
#define PCI_VENDOR_ID            0x00
#define PCI_DEVICE_ID            0x02
#define PCI_COMMAND              0x04
#define PCI_STATUS               0x06
#define PCI_REVISION_ID          0x08
#define PCI_PROG_IF              0x09
#define PCI_SUBCLASS             0x0a
#define PCI_CLASS                0x0b
#define PCI_CACHE_LINE_SIZE      0x0c
#define PCI_LATENCY_TIMER        0x0d
#define PCI_HEADER_TYPE          0x0e
#define PCI_BIST                 0x0f
#define PCI_BAR0                 0x10
#define PCI_BAR1                 0x14
#define PCI_BAR2                 0x18
#define PCI_BAR3                 0x1C
#define PCI_BAR4                 0x20
#define PCI_BAR5                 0x24
#define PCI_INTERRUPT_LINE       0x3C
#define PCI_SECONDARY_BUS        0x09

// Device type
#define PCI_HEADER_TYPE_DEVICE  0
#define PCI_HEADER_TYPE_BRIDGE  1
#define PCI_HEADER_TYPE_CARDBUS 2
#define PCI_TYPE_BRIDGE 0x0604
#define PCI_TYPE_SATA   0x0106
#define PCI_NONE 0xFFFF

#define DEVICE_PER_BUS           32
#define FUNCTION_PER_DEVICE      32

uint32_t pci_read(pci_dev_t dev, uint32_t field);
void pci_write(pci_dev_t dev, uint32_t field, uint32_t value);
uint32_t get_device_type(pci_dev_t dev);
uint32_t get_secondary_bus(pci_dev_t dev);
uint32_t pci_reach_end(pci_dev_t dev);
pci_dev_t pci_scan_function(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, uint32_t function, int device_type);
pci_dev_t pci_scan_device(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, int device_type);
pci_dev_t pci_scan_bus(uint16_t vendor_id, uint16_t device_id, uint32_t bus, int device_type);
pci_dev_t pci_get_device(uint16_t vendor_id, uint16_t device_id, int device_type);
void pci_init();

uint32_t pci_size_map[100];
pci_dev_t dev_zero= {0};

uint32_t pci_read(pci_dev_t dev, uint32_t field) {
	// Only most significant 6 bits of the field
	dev.field_num = (field & 0xFC) >> 2;
	dev.enable = 1;
	outportl(PCI_CONFIG_ADDRESS, dev.bits);

	// What size is this field supposed to be ?
	uint32_t size = pci_size_map[field];
	if(size == 1) {
		// Get the first byte only, since it's in little endian, it's actually the 3rd byte
		uint8_t t =inportb(PCI_CONFIG_DATA + (field & 3));
		return t;
	}
	else if(size == 2) {
		uint16_t t = inports(PCI_CONFIG_DATA + (field & 2));
		return t;
	}
	else if(size == 4){
		// Read entire 4 bytes
		uint32_t t = inportl(PCI_CONFIG_DATA);
		return t;
	}
	return 0xffff;
}

void pci_write(pci_dev_t dev, uint32_t field, uint32_t value) {
	dev.field_num = (field & 0xFC) >> 2;
	dev.enable = 1;
	// Tell where we want to write
	outportl(PCI_CONFIG_ADDRESS, dev.bits);
	// Value to write
	outportl(PCI_CONFIG_DATA, value);
}

uint32_t get_device_type(pci_dev_t dev) {
	uint32_t t = pci_read(dev, PCI_CLASS) << 8;
	return t | pci_read(dev, PCI_SUBCLASS);
}

uint32_t get_secondary_bus(pci_dev_t dev) {
	return pci_read(dev, PCI_SECONDARY_BUS);
}

uint32_t pci_reach_end(pci_dev_t dev) {
	uint32_t t = pci_read(dev, PCI_HEADER_TYPE);
	return !t;
}

pci_dev_t pci_scan_function(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, uint32_t function, int device_type) {
	pci_dev_t dev = {0};
	dev.bus_num = bus;
	dev.device_num = device;
	dev.function_num = function;
	// If it's a PCI Bridge device, get the bus it's connected to and keep searching
	if(get_device_type(dev) == PCI_TYPE_BRIDGE) {
		pci_scan_bus(vendor_id, device_id, get_secondary_bus(dev), device_type);
	}
	// If type matches, we've found the device, just return it
	if(device_type == -1 || (uint32_t) device_type == get_device_type(dev)) {
		uint32_t devid  = pci_read(dev, PCI_DEVICE_ID);
		uint32_t vendid = pci_read(dev, PCI_VENDOR_ID);
		if(devid == device_id && vendor_id == vendid)
			return dev;
	}
	return dev_zero;
}

pci_dev_t pci_scan_device(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, int device_type) {
	pci_dev_t dev = {0};
	dev.bus_num = bus;
	dev.device_num = device;

	if(pci_read(dev,PCI_VENDOR_ID) == PCI_NONE)
		return dev_zero;

	pci_dev_t t = pci_scan_function(vendor_id, device_id, bus, device, 0, device_type);
	if(t.bits)
		return t;

	if(pci_reach_end(dev))
		return dev_zero;

	for(int function = 1; function < FUNCTION_PER_DEVICE; function++) {
		if(pci_read(dev,PCI_VENDOR_ID) != PCI_NONE) {
			t = pci_scan_function(vendor_id, device_id, bus, device, function, device_type);
			if(t.bits)
				return t;
		}
	}
	return dev_zero;
}

pci_dev_t pci_scan_bus(uint16_t vendor_id, uint16_t device_id, uint32_t bus, int device_type) {
	for(int device = 0; device < DEVICE_PER_BUS; device++) {
		pci_dev_t t = pci_scan_device(vendor_id, device_id, bus, device, device_type);
		if(t.bits)
			return t;
	}
	return dev_zero;
}

pci_dev_t pci_get_device(uint16_t vendor_id, uint16_t device_id, int device_type) {

	pci_dev_t t = pci_scan_bus(vendor_id, device_id, 0, device_type);
	if(t.bits)
		return t;

	// Handle multiple pci host controllers

	if(pci_reach_end(dev_zero)) {
		if (DEBUG) {kprintf("PCI Get device failed...\n");}
	}
	for(int function = 1; function < FUNCTION_PER_DEVICE; function++) {
		pci_dev_t dev = {0};
		dev.function_num = function;

		if(pci_read(dev, PCI_VENDOR_ID) == PCI_NONE)
			break;
		t = pci_scan_bus(vendor_id, device_id, function, device_type);
		if(t.bits)
			return t;
	}
	return dev_zero;
}

void pci_init() {
	// Init size map
	pci_size_map[PCI_VENDOR_ID] =	2;
	pci_size_map[PCI_DEVICE_ID] =	2;
	pci_size_map[PCI_COMMAND]	=	2;
	pci_size_map[PCI_STATUS]	=	2;
	pci_size_map[PCI_SUBCLASS]	=	1;
	pci_size_map[PCI_CLASS]		=	1;
	pci_size_map[PCI_CACHE_LINE_SIZE]	= 1;
	pci_size_map[PCI_LATENCY_TIMER]		= 1;
	pci_size_map[PCI_HEADER_TYPE] = 1;
	pci_size_map[PCI_BIST] = 1;
	pci_size_map[PCI_BAR0] = 4;
	pci_size_map[PCI_BAR1] = 4;
	pci_size_map[PCI_BAR2] = 4;
	pci_size_map[PCI_BAR3] = 4;
	pci_size_map[PCI_BAR4] = 4;
	pci_size_map[PCI_BAR5] = 4;
	pci_size_map[PCI_INTERRUPT_LINE]	= 1;
	pci_size_map[PCI_SECONDARY_BUS]		= 1;
}

// RTL NOW

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

void rtl8139_handler(registers_t* reg) {
    if (DEBUG) {kprintf("RTL8139 interript was fired !!!! \n");}
    uint16_t status = inports(rtl8139_device.io_base + 0x3e);

    if(status & TOK) {
        if (DEBUG) {kprintf("Packet sent\n");}
    }
    if (status & ROK) {
        if (DEBUG) {kprintf("Received packet\n");}
        //receive_packet();
    }

    outports(rtl8139_device.io_base + 0x3E, 0x5);
}

void rtl8139_init() {
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

int ethernet_init() {
    pci_init();
    rtl8139_init();
    return 0;
}

void ethernet_call(int thing) {
    switch(thing) {
        case 0:
            read_mac_addr();
            break;
        default:
            return;
    }
}
