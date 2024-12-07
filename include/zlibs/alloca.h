/*****************************************************************************\
|   === alloca.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the alloca.h header file                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _ALLOCA_H
#define _ALLOCA_H

#include <stddef.h>

#undef alloca

void *alloca(size_t size);

#ifdef __GNUC__ // GCC
#define alloca(size) __builtin_alloca(size)
#endif

#endif
