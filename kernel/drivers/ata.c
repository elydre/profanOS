#include <drivers/ata.h>
#include <cpu/ports.h>

/*
BSY: a 1 means that the controller is busy executing a command. No register should be accessed (except the digital output register) while this bit is set.
RDY: a 1 means that the controller is ready to accept a command, and the drive is spinning at correct speed..
WFT: a 1 means that the controller detected a write fault.
SKC: a 1 means that the read/write head is in position (seek completed).
DRQ: a 1 means that the controller is expecting data (for a write) or is sending data (for a read). Don't access the data register while this bit is 0.
COR: a 1 indicates that the controller had to correct data, by using the ECC bytes (error correction code: extra bytes at the end of the sector that allows to verify its integrity and, sometimes, to correct errors).
IDX: a 1 indicates the the controller retected the index mark (which is not a hole on hard-drives).
ERR: a 1 indicates that an error occured. An error code has been placed in the error register.
*/

#define STATUS_BSY 0x80
#define STATUS_RDY 0x40
#define STATUS_DRQ 0x08
#define STATUS_DF  0x20
#define STATUS_ERR 0x01

// Source - OsDev wiki
static void ATA_wait_BSY(void);
static void ATA_wait_DRQ(void);


void ata_write_sector(uint32_t LBA, uint32_t *data) {
    // LBA : Logical Block Address
    // data: uint32_t data[128]
    ATA_wait_BSY();
    ATA_wait_DRQ();

    port_byte_out(0x1F6, 0xE0 | ((LBA >> 24) & 0x0F));
    port_byte_out(0x1F1, 0x00);
    port_byte_out(0x1F2, 0x01);
    port_byte_out(0x1F3, (uint8_t) LBA);
    port_byte_out(0x1F4, (uint8_t) (LBA >> 8));
    port_byte_out(0x1F5, (uint8_t) (LBA >> 16));
    port_byte_out(0x1F7, 0x30);

    ATA_wait_BSY();
    ATA_wait_DRQ();

    for (int i = 0; i < 128; i++) {
        port_long_out(0x1F0, data[i]);
    }
}

void ata_read_sector(uint32_t LBA, uint32_t *data) {
    // LBA : Logical Block Address
    // data: uint32_t data[128]
    ATA_wait_BSY();
    ATA_wait_DRQ();

    port_byte_out(0x1F6, 0xE0 | ((LBA >> 24) & 0x0F));
    port_byte_out(0x1F1, 0x00);
    port_byte_out(0x1F2, 0x01);
    port_byte_out(0x1F3, (uint8_t) LBA);
    port_byte_out(0x1F4, (uint8_t) (LBA >> 8));
    port_byte_out(0x1F5, (uint8_t) (LBA >> 16));
    port_byte_out(0x1F7, 0x20);

    ATA_wait_BSY();
    ATA_wait_DRQ();

    for (int i = 0; i < 128; i++) {
        data[i] = port_long_in(0x1F0);
    }
}

uint32_t ata_get_sectors_count(void) {
    // return 0 if ata is not present
    for (int count = 0; port_byte_in(0x1F7) & STATUS_BSY; count++) {
        if (count > 0x1000) return 0;
    }
    port_byte_out(0x1F6,0xE0 | ((0 >> 24) & 0xF));
    port_byte_out(0x1F2, 0);
    port_byte_out(0x1F3, 0);
    port_byte_out(0x1F4, 0);
    port_byte_out(0x1F5, 0);
    port_byte_out(0x1F7, 0xEC); // send the identify command

    if (port_byte_in(0x1F7) == 0) return 0;

    ATA_wait_BSY();
    ATA_wait_DRQ();

    uint16_t bytes[256];
    for (int i = 0; i < 256; i++) {
        bytes[i] = port_word_in(0x1F0);
    }

    uint32_t size = bytes[61] << 16 | bytes[60];
    return size;
}

static void ATA_wait_BSY(void) {    // wait for bsy to be 0
    while (port_byte_in(0x1F7) & STATUS_BSY);
}

static void ATA_wait_DRQ(void) {    // wait fot drq to be 1
    while (!(port_byte_in(0x1F7) & STATUS_RDY));
}
