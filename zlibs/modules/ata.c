/*****************************************************************************\
|   === ata.c : 2024 ===                                                      |
|                                                                             |
|    ATA driver implementation as kernel module                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Based on OsDev wiki - wiki.osdev.org/ATA_PIO_Mode             `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <kernel/afft.h>
#include <cpu/ports.h>
#include <minilib.h>

/*
BSY: a 1 means that the controller is busy executing a command.
     No register should be accessed (except the digital output register) while this bit is set.
RDY: a 1 means that the controller is ready to accept a command, and the drive is spinning at correct speed..
WFT: a 1 means that the controller detected a write fault.
SKC: a 1 means that the read/write head is in position (seek completed).
DRQ: a 1 means that the controller is expecting data (for a write) or is sending data (for a read).
     Don't access the data register while this bit is 0.
COR: a 1 indicates that the controller had to correct data, by using the ECC bytes (error correction code:
     extra bytes at the end of the sector that allows to verify its integrity and, sometimes, to correct errors).
IDX: a 1 indicates the the controller retected the index mark (which is not a hole on hard-drives).
ERR: a 1 indicates that an error occured. An error code has been placed in the error register.
*/

#define STATUS_BSY 0x80
#define STATUS_RDY 0x40
#define STATUS_DRQ 0x08
#define STATUS_DF  0x20
#define STATUS_ERR 0x01

static void ATA_wait_BSY(void) {    // wait for bsy to be 0
    while (port_read8(0x1F7) & STATUS_BSY);
}

static void ATA_wait_DRQ(void) {    // wait fot drq to be 1
    while (!(port_read8(0x1F7) & STATUS_RDY));
}

static void write_sector(uint32_t LBA, uint32_t *data) {
    ATA_wait_BSY();
    ATA_wait_DRQ();

    port_write8(0x1F6, 0xE0 | ((LBA >> 24) & 0x0F));
    port_write8(0x1F1, 0x00);
    port_write8(0x1F2, 0x01);
    port_write8(0x1F3, (uint8_t) LBA);
    port_write8(0x1F4, (uint8_t) (LBA >> 8));
    port_write8(0x1F5, (uint8_t) (LBA >> 16));
    port_write8(0x1F7, 0x30);

    ATA_wait_BSY();
    ATA_wait_DRQ();

    for (int i = 0; i < 128; i++) {
        port_write32(0x1F0, data[i]);
    }
}

static void read_sector(uint32_t LBA, uint32_t *data) {
    ATA_wait_BSY();
    ATA_wait_DRQ();

    port_write8(0x1F6, 0xE0 | ((LBA >> 24) & 0x0F));
    port_write8(0x1F1, 0x00);
    port_write8(0x1F2, 0x01);
    port_write8(0x1F3, (uint8_t) LBA);
    port_write8(0x1F4, (uint8_t) (LBA >> 8));
    port_write8(0x1F5, (uint8_t) (LBA >> 16));
    port_write8(0x1F7, 0x20);

    ATA_wait_BSY();
    ATA_wait_DRQ();

    for (int i = 0; i < 128; i++) {
        data[i] = port_read32(0x1F0);
    }
}


int ata_write(uint32_t id, void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(id);

    if (size % 512 != 0 || offset % 512 != 0)
        return -1;

    uint32_t LBA = offset / 512;
    uint32_t *data = buffer;

    size /= 512;

    for (uint32_t i = 0; i < size; i++) {
        write_sector(LBA + i, data);
        data += 128;
    }

    return size;
}

int ata_read(uint32_t id, void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(id);

    if (size % 512 != 0 || offset % 512 != 0)
        return -1;

    uint32_t LBA = offset / 512;
    uint32_t *data = buffer;

    size /= 512;

    for (uint32_t i = 0; i < size; i++) {
        read_sector(LBA + i, data);
        data += 128;
    }

    return size;
}

int ata_get_sectors_count(void) {
    for (int count = 0; port_read8(0x1F7) & STATUS_BSY; count++) {
        if (count > 100) // timeout, no drive
            return -1;
    }

    port_write8(0x1F6,0xE0 | ((0 >> 24) & 0xF));
    port_write8(0x1F2, 0);
    port_write8(0x1F3, 0);
    port_write8(0x1F4, 0);
    port_write8(0x1F5, 0);
    port_write8(0x1F7, 0xEC); // send the identify command

    if (port_read8(0x1F7) == 0) // no drive
        return -1;

    ATA_wait_BSY();
    ATA_wait_DRQ();

    uint16_t bytes[256];

    for (int i = 0; i < 256; i++)
        bytes[i] = port_read16(0x1F0);

    return bytes[61] << 16 | bytes[60];
}

int __init(void) {
    int afft_id;

    if (ata_get_sectors_count() == -1)
        return 2; // pass, no drive

    afft_id = afft_register(AFFT_AUTO, ata_read, ata_write, NULL);

    if (afft_id == -1 || kfu_afft_create("/dev", "ata", afft_id) == SID_NULL)
        return 1;

    return 0;
}

void *__module_func_array[] = {
    (void *) 0xF3A3C4D4, // magic
    // no functions exported
};
