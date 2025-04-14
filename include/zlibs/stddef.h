/*****************************************************************************\
|   === stddef.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the stddef.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original header - mintsuki/freestanding-headers               `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _STDDEF_H
#define _STDDEF_H

typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201710L)
  typedef typeof(nullptr) nullptr_t;
#endif

#endif

#ifdef __cplusplus
  typedef void* nullptr_t;
  typedef long double max_align_t;
#endif

#undef NULL
#ifndef __cplusplus
  #define NULL ((void *) 0)
#else
  #define NULL 0
#endif

#undef offsetof
#define offsetof(s, m) __builtin_offsetof(s, m)

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201710L)
  #undef unreachable
  #define unreachable() __builtin_unreachable()
#endif

#endif
