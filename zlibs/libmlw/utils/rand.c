/*****************************************************************************\
|   === rand.c : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "../mlw_private.h"

uint16_t I_mlw_rand_u16() {
    return rand() * 1.0 / RAND_MAX * 0xFFFF;
}

uint32_t I_mlw_rand_u32() {
    return (uint32_t)rand();
}
