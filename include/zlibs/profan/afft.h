/*****************************************************************************\
|   === afft.h : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_AFFT_H
#define _PROFAN_AFFT_H

#include <profan/minimal.h>

_BEGIN_C_FILE

#define AFFT_MAX      256
#define AFFT_RESERVED 16
#define AFFT_NAME_MAX 32

// commands with 2272 prefix are reserved for profanOS stuff
#define AFFTC_EXISTS  0x2272000 // return 1 if exists, 0 if not
#define AFFTC_GETNAME 0x2272001 // copy name to arg (char arg[32])

_END_C_FILE

#endif
