/*****************************************************************************\
|   === ahci.h : 2025 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef AHCI_MODULE_ID
#define AHCI_MODULE_ID 7

#include <stdint.h>

uint8_t ahci_read_sectors(uint16_t drive_num, uint64_t start_sector, uint32_t count, void *buf);
uint8_t ahci_write_sectors(uint16_t drive_num, uint64_t start_sector, uint32_t count, void *buf);
int drive_exists(uint16_t drive_num);


#ifndef _KERNEL_MODULE
// access depuis l'espace utilisateur

extern int profan_syscall(uint32_t id, ...);

#define _pscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

#define ahci_read_sectors(a, b, c, d) ((uint8_t) _pscall(AHCI_MODULE_ID, 0, a, (uint64_t) b, c, d))
#define ahci_write_sectors(a, b, c, d) ((uint8_t) _pscall(AHCI_MODULE_ID, 1, a, (uint64_t) b, c, d))
#define drive_exists(a) ((int) _pscall(AHCI_MODULE_ID, 2, a))

#endif // _KERNEL_MODULE

#endif
