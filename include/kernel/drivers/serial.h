/*****************************************************************************\
|   === serial.h : 2024 ===                                                   |
|                                                                             |
|    Kernel Serial driver header                                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef SERIAL_H
#define SERIAL_H

#include <ktype.h>

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8


int serial_init(void);
int serial_write(int device, char *buf, uint32_t len);
int serial_read(int device, char *buf, uint32_t len);

#endif
