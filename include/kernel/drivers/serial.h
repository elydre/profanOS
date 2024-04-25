/****** This file is part of profanOS **************************\
|   == serial.h ==                                   .pi0iq.    |
|                                                   d"  . `'b   |
|                                                   q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef SERIAL_H
#define SERIAL_H

#include <ktype.h>

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8


int  serial_init(void);
void serial_write(int device, char *buf, uint32_t len);
void serial_read(int device, char *buf, uint32_t len);

#endif
