/*****************************************************************************\
|   === afft.h : 2025 ===                                                     |
|                                                                             |
|    Kernel advanced file system functions header                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef AFFT_H
#define AFFT_H

#include <ktype.h>

#define AFFT_MAX      256
#define AFFT_RESERVED 16

#define AFFT_AUTO 0xFFFFFFFF

int afft_init(void);

int afft_register(
        uint32_t wanted_id,
        int (*read)  (uint32_t id, void *buffer, uint32_t offset, uint32_t size),
        int (*write) (uint32_t id, void *buffer, uint32_t offset, uint32_t size),
        int (*cmd)   (uint32_t id, uint32_t cmd, void *arg)
);

int afft_read  (uint32_t id, void *buffer, uint32_t offset, uint32_t size);
int afft_write (uint32_t id, void *buffer, uint32_t offset, uint32_t size);
int afft_cmd   (uint32_t id, uint32_t cmd, void *arg);

#endif
