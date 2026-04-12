/*****************************************************************************\
|   === endian.h : 2026 ===                                                   |
|                                                                             |
|    Implementation of the endian.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original code from freeBSD (see below)                        `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _ENDIAN_H
#define _ENDIAN_H

#include <profan/minimal.h>

_BEGIN_C_FILE

// Define the order of 32-bit words in 64-bit words.
#define _QUAD_HIGHWORD 1
#define _QUAD_LOWWORD 0

// Definitions for byte order, according to byte significance from low
// address to high.

#ifndef LITTLE_ENDIAN
  #define LITTLE_ENDIAN 1234    // LSB first: i386, vax
#endif

#ifndef BIG_ENDIAN
  #define BIG_ENDIAN    4321    // MSB first: 68000, ibm, net
#endif

#ifndef PDP_ENDIAN
  #define PDP_ENDIAN    3412    // LSB first in word, MSW first in long
#endif

#ifndef BYTE_ORDER
  #define BYTE_ORDER    LITTLE_ENDIAN
#endif

#include <sys/cdefs.h>
#include <sys/types.h>

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

#define NTOHL(x) ((x) = ntohl((uint32_t)(x)))
#define NTOHS(x) ((x) = ntohs((uint16_t)(x)))
#define HTONL(x) ((x) = htonl((uint32_t)(x)))
#define HTONS(x) ((x) = htons((uint16_t)(x)))

_END_C_FILE

#endif
