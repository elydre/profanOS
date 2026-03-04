/*****************************************************************************\
|   === ahci.h : 2025 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef FS_AHCI_H
#define FS_AHCI_H

#define HBA_DET_PRESENT     3
#define HBA_IPM_ACTIVE      1
#define SATA_READ_DMA_EX    0x25
#define SATA_WRITE_DMA_EX   0x35
#define SATA_IDENTIFY_DEVICE 0xEC
#define HBA_CMD_CR          (1 << 15)
#define HBA_CMD_FR          (1 << 14)
#define HBA_CMD_FRE         (1 << 4)
#define HBA_CMD_SUD         (1 << 1)
#define HBA_CMD_ST          (1)
#define SATA_BUSY           0x80
#define SATA_DRQ            0x08

typedef volatile struct {
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t res0;
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sctl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
    uint32_t sntf;
    uint32_t fbs;
    uint32_t res1[11];
    uint32_t vs[4];
} HBAPort;

typedef volatile struct {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_ports;
    uint32_t em_loc;
    uint32_t em_ctl;
    uint32_t cap2;
    uint32_t bohc;

    uint8_t reserved[116];

    uint8_t vendorRegisters[96];

    HBAPort ports[1];
} HBAData;

typedef struct {
    uint8_t FIS_Type;

    uint8_t pmport : 4;
    uint8_t reserved0 : 3;
    uint8_t c : 1;

    uint8_t command;
    uint8_t feature_low;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t dev;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t feature_high;

    uint8_t countl;
    uint8_t counth;
    uint8_t isync_cmd_complete;
    uint8_t ctrl_reg;

    uint32_t reserved1;
} FIS_HostToDevice;

typedef struct {
    uint8_t fis_type;

    uint8_t portmul : 4;
    uint8_t reserved0 : 2;
    uint8_t interrupt_bit : 1;
    uint8_t reserved1 : 1;

    uint8_t status_reg;
    uint8_t error_reg;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t dev;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t reserved2;

    uint8_t  count_low;
    uint8_t  count_high;
    uint16_t reserved3;

    uint32_t reserved4;
} FIS_DeviceToHost;

typedef struct {
    uint8_t fis_type;

    uint8_t  portmul : 4;
    uint8_t  reserved0 : 4;
    uint16_t reserved1;

    uint32_t data[1];
} FIS_Data;

typedef struct {
    uint8_t fis_type;

    uint8_t portmul : 4;
    uint8_t reserved0 : 1;
    uint8_t data_direction : 1;
    uint8_t interrupt_bit : 1;
    uint8_t reserved1 : 1;

    uint8_t status_reg;
    uint8_t error_reg;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t dev;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t reserved2;

    uint8_t count_low;
    uint8_t count_high;
    uint8_t reserved3;
    uint8_t e_status;

    uint16_t transfer_count;
    uint16_t reserved4;
} FIS_PIO_Setup;

typedef struct {
    uint8_t fis_type;

    uint8_t portmul : 4;
    uint8_t reserved0 : 1;
    uint8_t d : 1;
    uint8_t i : 1;
    uint8_t a : 1;

    uint8_t reserved1;

    uint64_t DMA_buffer_id;

    uint32_t reserved2;

    uint32_t DMA_buffer_offset;

    uint32_t transfer_count;

    uint32_t reserved3;

} FIS_DMA_Setup;

typedef volatile struct {
    FIS_DMA_Setup DMASetup;
    uint8_t       pad0[4];

    FIS_PIO_Setup PIOSetup;
    uint8_t       pad1[12];

    FIS_DeviceToHost RegDeviceToHost;
    uint8_t          pad2[4];

    uint16_t dev_bits;

    uint8_t ufis[64];

    uint8_t reserved[96];
} HBA_FIS;

typedef struct {
    uint8_t cfl : 5;
    uint8_t a : 1;
    uint8_t w : 1;
    uint8_t p : 1;
    uint8_t r : 1;
    uint8_t b : 1;
    uint8_t c : 1;
    uint8_t reserved0 : 1;
    uint8_t pmp : 4;

    uint16_t prdtl;

    volatile uint32_t prdbc;

    uint32_t ctba;
    uint32_t ctbau;

    uint32_t reserved1[4];
} HBACommandHeader;

typedef struct {
    uint32_t dba;
    uint32_t dbau;
    uint32_t reserved0;

    uint32_t dbc : 22;
    uint32_t reserved1 : 9;
    uint32_t i : 1;
} HBAPhysRegionDT;

typedef struct {
    uint8_t cfis[64];

    uint8_t acmd[16];

    uint8_t res0[48];

    HBAPhysRegionDT prdt_entry[1];
} HBACommandTable;

typedef struct {
    uint16_t gen_cfg;
    uint16_t unused0[9];
    char serial_number[20];
    uint16_t unused1[3];
    char firmware_rev[8];
    char model_number[40];
    uint16_t sectors_per_int;
    uint16_t reserved0;
    uint16_t capabilities[2];
    uint16_t unused2[2];
    uint16_t valid_values;
    uint16_t unused3[5];
    uint16_t multi_sector;
    uint32_t user_addressable_sectors;
    uint16_t unused4[13];
    uint16_t max_queue_depth;
    uint64_t reserved1;
    uint16_t major_version;
    uint16_t minor_version;
    uint32_t command_sets_supported;
    uint16_t command_extension_supported;
    uint32_t command_sets_enabled;
    uint16_t command_set_default;
    uint16_t ultra_dma;
    uint16_t security_erase_time;
    uint16_t e_security_erase_time;
    uint16_t power_mgmt_val;
    uint16_t master_password_rev;
    uint16_t hw_reset_result;
    uint16_t acoustic_mgmt;
    uint16_t streaming[5];
    uint64_t total_sectors;
    uint32_t unused5;
    uint16_t logical_sector_size;
    uint16_t unused6[10];
    uint32_t words_per_logical_sector;
    uint16_t unused7[136];
    uint16_t checksum;
} __attribute__((packed)) sata_identify_packet;

void    init_ahci();
uint8_t ahci_read_sectors(uint16_t drive_num, uint64_t start_sector, uint32_t count, void *buf);
uint8_t ahci_write_sectors(uint16_t drive_num, uint64_t start_sector, uint32_t count, void *buf);
int     drive_exists(uint16_t drive_num);
void    print_sector(uint8_t *read);

#endif
