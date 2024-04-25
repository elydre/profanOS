/****** This file is part of profanOS **************************\
|   == assert.h ==                                   .pi0iq.    |
|                                                   d"  . `'b   |
|   Implementation assert.h header file             q. /|\ .p   |
|   for the C standard library                       '// \\'    |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef ASSERT_H
#define ASSERT_H

void __assert_fail(const char *expr, const char *file, int line, const char *func);

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#define assert(expr) ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__, __func__))
#endif

#endif
