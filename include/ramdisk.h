#ifndef RAMDISK_H
#define RAMDISK_H

void ramdisk_init();
void ramdisk_read_sector(uint32_t LBA, uint32_t out[]);
void ramdisk_write_sector(uint32_t LBA, uint32_t bytes[]);

#endif
