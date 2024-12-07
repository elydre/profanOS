/*****************************************************************************\
|   === libtsi.h : 2024 ===                                                   |
|                                                                             |
|    Terminal based scrollable interface library header            .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_LIBTSI_H
#define _PROFAN_LIBTSI_H

#include <stdint.h>

#undef TSI_ALLOW_NON_PRINTABLE
#define TSI_ALLOW_NON_PRINTABLE 0x01

int tsi_start(const char *title, const char *string, uint32_t flags);
int tsi_start_array(const char *title, const char **lines, uint32_t flags);

#endif
