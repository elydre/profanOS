/*****************************************************************************\
|   === setjmp.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the setjmp.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _SETJMP_H
#define _SETJMP_H

#include <profan/minimal.h>

_BEGIN_C_FILE

typedef int jmp_buf[6];

int setjmp(jmp_buf var) __attribute__((returns_twice));
void longjmp(jmp_buf var, int m) __attribute__((noreturn));

_END_C_FILE

#endif
