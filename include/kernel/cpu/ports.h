/*****************************************************************************\
|   === ports.h : 2024 ===                                                    |
|                                                                             |
|    Kernel Input/Output ports header                              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef PORTS_H
#define PORTS_H

#include <ktype.h>

uint8_t  port_byte_in(uint16_t port);
void     port_byte_out(uint16_t port, uint8_t data);

uint16_t port_word_in(uint16_t port);
void     port_word_out(uint16_t port, uint16_t data);

uint32_t port_long_in(uint32_t port);
void     port_long_out(uint32_t port, uint32_t value);

#endif
