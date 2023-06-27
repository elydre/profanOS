#ifndef RAMDISK_H
#define RAMDISK_H

#include <type.h>

int ramdisk_init();

void ramdisk_read_sector(uint32_t LBA, uint32_t out[]);
void ramdisk_write_sector(uint32_t LBA, uint32_t bytes[]);

uint32_t ramdisk_get_address();


/***********************
 * RAMDISK INFO CODES *
 *    info 0: size    *
 *    info 1: used    *
***********************/

uint32_t ramdisk_get_info(int info);

#endif
