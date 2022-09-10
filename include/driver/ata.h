#ifndef ATA_H
#define ATA_H

#include <stdint.h>

void read_sectors_ATA_PIO(uint32_t LBA, uint32_t out[]);
void write_sectors_ATA_PIO(uint32_t LBA, uint32_t bytes[]);
uint32_t get_ATA_sectors_count();

#endif
