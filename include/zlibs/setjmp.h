/****** This file is part of profanOS **************************\
|   == setjmp.h ==                                   .pi0iq.    |
|                                                   d"  . `'b   |
|   Implementation of setjmp.h header file          q. /|\  u   |
|   for the C standard library                       `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef SETJMP_H
#define SETJMP_H

typedef int jmp_buf[6];

int setjmp(jmp_buf var);
void longjmp(jmp_buf var, int m);

#endif
