#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdint.h>

void ramdisk_init();
void ramdisk_read_sector(uint32_t LBA, uint32_t out[]);
void ramdisk_write_sector(uint32_t LBA, uint32_t bytes[]);

uint32_t ramdisk_get_address();

// size and used are in sectors
uint32_t ramdisk_get_size();
uint32_t ramdisk_get_used();

#endif
