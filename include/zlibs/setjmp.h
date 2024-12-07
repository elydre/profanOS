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

typedef int jmp_buf[6];

int setjmp(jmp_buf var);
void longjmp(jmp_buf var, int m) __attribute__((noreturn));

#endif
