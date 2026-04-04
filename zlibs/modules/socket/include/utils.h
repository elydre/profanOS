/*****************************************************************************\
|   === utils.h : 2026 ===                                                    |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef UTILS_SOCK_H
#define UTILS_SOCK_H

#include <stdint.h>

uint16_t ntohs(uint16_t x);
uint32_t ntohl(uint32_t x);

#define htons ntohs
#define htonl ntohl

#endif
