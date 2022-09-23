#ifndef ATA_H
#define ATA_H

#include <stdint.h>

void ata_read_sector(uint32_t LBA, uint32_t out[]);
void ata_write_sector(uint32_t LBA, uint32_t bytes[]);
uint32_t ata_get_sectors_count();

#endif
