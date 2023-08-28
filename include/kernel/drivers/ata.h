#ifndef ATA_H
#define ATA_H

#include <ktype.h>

void ata_write_sector(uint32_t LBA, uint32_t *data);
void ata_read_sector(uint32_t LBA, uint32_t *data);

uint32_t ata_get_sectors_count();

#endif
