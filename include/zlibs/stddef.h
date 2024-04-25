/****** This file is part of profanOS **************************\
|   == stddef.h ==                                   .pi0iq.    |
|                                                   d"  . `'b   |
|   Implementation of stddef.h header file          q. /|\ .p   |
|   see mintsuki/freestanding-headers                '// \\'    |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#if !defined(__FSTD_HDRS_STDDEF_H) || defined(__FSTD_HDRS_CXX_WRAP)
#if !defined(__FSTD_HDRS_CXX_WRAP)
#define __FSTD_HDRS_STDDEF_H 1
#endif

typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;

/* XXX fix to use proper __STDC_VERSION__ number later on */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202000L)
typedef typeof(nullptr) nullptr_t;
#endif

#endif

#ifdef __cplusplus
typedef decltype(nullptr) nullptr_t;
#endif

#undef NULL
#ifndef __cplusplus
#  define NULL ((void *)0)
#else
#  define NULL 0
#endif

#undef offsetof
#define offsetof(s, m) __builtin_offsetof(s, m)

/* XXX fix to use proper __STDC_VERSION__ number later on */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202000L)
#  undef unreachable
#  define unreachable() __builtin_unreachable()
#endif

#endif
