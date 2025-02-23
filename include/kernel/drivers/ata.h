/*****************************************************************************\
|   === ata.h : 2024 ===                                                      |
|                                                                             |
|    Kernel ATA driver header                                      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef ATA_H
#define ATA_H

#include <ktype.h>

void     ata_write_sector(uint32_t LBA, uint32_t *data);
void     ata_read_sector(uint32_t LBA, uint32_t *data);

uint32_t ata_get_sectors_count(void);

#endif
