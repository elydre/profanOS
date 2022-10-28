#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdint.h>

void ramdisk_init();
void ramdisk_read_sector(uint32_t LBA, uint32_t out[]);
void ramdisk_write_sector(uint32_t LBA, uint32_t bytes[]);

uint32_t get_ramdisk_address();
uint32_t get_ramdisk_size();

#endif
