/*****************************************************************************\
|   === stdarg.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the stdarg.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original header - mintsuki/freestanding-headers               `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _STDARG_H
#define _STDARG_H

typedef __builtin_va_list va_list;

#undef  va_start
#define va_start(v, l) __builtin_va_start(v, l)

#undef  va_end
#define va_end(v) __builtin_va_end(v)

#undef  va_arg
#define va_arg(v, l) __builtin_va_arg(v, l)

#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
  #undef  va_copy
  #define va_copy(d, s) __builtin_va_copy(d, s)
#endif

#endif
