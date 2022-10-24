#include <driver/ata.h>

#define RAMDISK_ADDR 0x1000000
#define UINT32_PER_SECTOR 128

void ramdisk_init() {
    // copy ata data to ramdisk
    uint32_t sectors_count = ata_get_sectors_count();
    // every sector save 128 uint32_t
    uint32_t* ramdisk = (uint32_t*)RAMDISK_ADDR;
    uint32_t sector[UINT32_PER_SECTOR];
    for (uint32_t i = 0; i < sectors_count; i++) {
        ata_read_sector(i, sector);
        for (uint32_t j = 0; j < UINT32_PER_SECTOR; j++) {
            ramdisk[i * UINT32_PER_SECTOR + j] = sector[j];
        }
    }
}

void ramdisk_read_sector(uint32_t LBA, uint32_t out[]) {
    uint32_t* ramdisk = (uint32_t*)RAMDISK_ADDR;
    for (uint32_t i = 0; i < UINT32_PER_SECTOR; i++) {
        out[i] = ramdisk[LBA * UINT32_PER_SECTOR + i];
    }
}

void ramdisk_write_sector(uint32_t LBA, uint32_t bytes[]) {
    uint32_t* ramdisk = (uint32_t*)RAMDISK_ADDR;
    for (uint32_t i = 0; i < UINT32_PER_SECTOR; i++) {
        ramdisk[LBA * UINT32_PER_SECTOR + i] = bytes[i];
    }
}
