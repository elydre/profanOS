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

uint32_t g_max_sector;

static void ATA_wait_BSY(void) {    // wait for bsy to be 0
    while (port_read8(0x1F7) & STATUS_BSY);
}

static void ATA_wait_DRQ(void) {    // wait fot drq to be 1
    while (!(port_read8(0x1F7) & STATUS_RDY));
}

static void write_sector(uint32_t LBA, uint8_t *data, uint32_t offset, uint32_t size) {
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

    uint8_t data_part[4];

    if (offset == 0 && size == 512) {
        // fast write
        kprintf_serial("fast write sector %d\n", LBA);
        for (int i = 0; i < 128; i++)
            port_write32(0x1F0, ((uint32_t *) data)[i]);
        return;
    }

    kprintf_serial("slow write sector %d\n", LBA);
    for (uint32_t i = 0; i < 512; i += 4) {
        for (uint32_t j = 0; j < 4; j++) {
            if (i + j < offset || i + j >= offset + size)
                data_part[j] = 0;
            else
                data_part[j] = data[i + j - offset];
        }

        port_write32(0x1F0, *(uint32_t *) data_part);
    }
}

static void read_sector(uint32_t LBA, uint8_t *data, uint32_t offset, uint32_t size) {
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

    uint8_t data_part[4];

    if (offset == 0 && size == 512) {
        // fast read
        kprintf_serial("fast read sector %d\n", LBA);
        for (int i = 0; i < 128; i++)
            ((uint32_t *) data)[i] = port_read32(0x1F0);
        return;
    }

    kprintf_serial("slow read sector %d\n", LBA);
    for (uint32_t i = 0; i < 512; i += 4) {
        *(uint32_t *) data_part = port_read32(0x1F0);

        for (uint32_t j = 0; j < 4; j++) {
            if (i + j < offset || i + j >= offset + size)
                continue;
            else
                data[i + j - offset] = data_part[j];
        }
    }
}


int ata_write(uint32_t id, void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(id);
    int res = size;

    uint32_t sector = offset / 512;

    if (sector >= g_max_sector)
        return -1;

    uint32_t offset_in_sector = offset % 512;
    uint32_t size_in_sector = 512 - offset_in_sector;

    if (size_in_sector > size)
        size_in_sector = size;

    write_sector(sector, buffer, offset_in_sector, size_in_sector);
    size -= size_in_sector;
    buffer = (uint8_t *) buffer + size_in_sector;
    offset += size_in_sector;

    while (size > 0 && sector < g_max_sector) {
        sector++;
        size_in_sector = size > 512 ? 512 : size;
        write_sector(sector, buffer, 0, size_in_sector);
        size -= size_in_sector;
        buffer = (uint8_t *) buffer + size_in_sector;
    }

    return res;
}

int ata_read(uint32_t id, void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(id);
    int res = size;

    uint32_t sector = offset / 512;

    if (sector >= g_max_sector)
        return -1;

    uint32_t offset_in_sector = offset % 512;
    uint32_t size_in_sector = 512 - offset_in_sector;

    if (size_in_sector > size)
        size_in_sector = size;

    read_sector(sector, buffer, offset_in_sector, size_in_sector);
    size -= size_in_sector;
    buffer = (uint8_t *) buffer + size_in_sector;
    offset += size_in_sector;

    while (size > 0 && sector < g_max_sector) {
        sector++;
        size_in_sector = size > 512 ? 512 : size;
        read_sector(sector, buffer, 0, size_in_sector);
        size -= size_in_sector;
        buffer = (uint8_t *) buffer + size_in_sector;
    }

    return res;
}

uint32_t ata_get_sectors_count(void) {
    for (int count = 0; port_read8(0x1F7) & STATUS_BSY; count++) {
        if (count > 100) // timeout, no drive
            return (uint32_t) -1;
    }

    port_write8(0x1F6,0xE0 | ((0 >> 24) & 0xF));
    port_write8(0x1F2, 0);
    port_write8(0x1F3, 0);
    port_write8(0x1F4, 0);
    port_write8(0x1F5, 0);
    port_write8(0x1F7, 0xEC); // send the identify command

    if (port_read8(0x1F7) == 0) // no drive
        return (uint32_t) -1;

    ATA_wait_BSY();
    ATA_wait_DRQ();

    uint16_t bytes[256];

    for (int i = 0; i < 256; i++)
        bytes[i] = port_read16(0x1F0);

    return bytes[61] << 16 | bytes[60];
}

int __init(void) {
    int afft_id;
    
    g_max_sector = ata_get_sectors_count();

    if (g_max_sector == (uint32_t) -1)
        return 2; // pass, no drive

    afft_id = afft_register(AFFT_AUTO, ata_read, ata_write, NULL, "ata");

    if (afft_id == -1 || kfu_afft_create("/dev", "ata", afft_id) == SID_NULL)
        return 1;

    return 0;
}

void *__module_func_array[] = {
    (void *) 0xF3A3C4D4, // magic
    // no functions exported
};
