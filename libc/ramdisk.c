#include <driver/ata.h>
#include <iolib.h>

#define RAMDISK_ADDR 0x1000000
#define UINT32_PER_SECTOR 128

uint32_t find_end_of_writed_sector() {
    uint32_t sector[UINT32_PER_SECTOR];
    int min = 0;
    int max = ata_get_sectors_count();
    int mid = (min + max) / 2;
    while (min < max) {
        ata_read_sector(mid, sector);
        if (sector[0] == 0) max = mid;
        else min = mid + 1;
        mid = (min + max) / 2;
    }
    return mid;
}

void ramdisk_init() {
    // copy ata data to ramdisk
    uint32_t sectors_count = find_end_of_writed_sector();
    // every sector save 128 uint32_t
    uint32_t* ramdisk = (uint32_t*)RAMDISK_ADDR;
    uint32_t sector[UINT32_PER_SECTOR];
    for (uint32_t i = 0; i < sectors_count; i++) {
        ata_read_sector(i, sector);
        for (uint32_t j = 0; j < UINT32_PER_SECTOR; j++) {
            ramdisk[i * UINT32_PER_SECTOR + j] = sector[j];
        }
        // display progress every 5percent
        if (i % (sectors_count / 20) == 0) {
            fskprint("Loading ramdisk %d/%d\r", i, sectors_count);
        }
    }
    fskprint("ramdisk initialized       \n");

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
