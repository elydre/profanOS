/*****************************************************************************\
|   === e1000.h : 2025 ===                                                    |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef E1000_H
#define E1000_H

#include <ktype.h>

extern int e1000_is_inited;

int e1000_send_packet(const void * p_data, uint16_t p_len);
int e1000_init(void);
void e1000_set_mac(uint8_t mac[6]);

#endif
