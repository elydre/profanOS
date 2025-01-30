/*****************************************************************************\
|   === limits.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the (32bit) limits.h header file            .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _LIMITS_H
#define _LIMITS_H

/**** CHAR_BIT *******************************************/

#undef CHAR_BIT

#ifdef __CHAR_BIT__
  #define CHAR_BIT __CHAR_BIT__
#else
  #define CHAR_BIT 8
#endif

/**** MB_LEN_MAX *****************************************/

#ifndef MB_LEN_MAX
  #define MB_LEN_MAX 1
#endif

/**** INT_MAX & INT_MIN **********************************/

#undef INT_MAX
#undef INT_MIN

#ifdef __INT_MAX__
  #define INT_MAX __INT_MAX__
#else
  #define INT_MAX 0x7FFFFFFF
#endif

#define INT_MIN (-INT_MAX - 1)

/**** UINT_MAX *******************************************/

#undef UINT_MAX
#define UINT_MAX (INT_MAX * 2U + 1U)

/**** SCHAR_MAX & SCHAR_MIN*******************************/

#undef SCHAR_MAX
#undef SCHAR_MIN

#ifdef __SCHAR_MAX__
  #define SCHAR_MAX __SCHAR_MAX__
#else
  #define SCHAR_MAX 0x7F
#endif

#define SCHAR_MIN (-SCHAR_MAX - 1)

/**** UCHAR_MAX ******************************************/

#undef UCHAR_MAX
#if SCHAR_MAX == INT_MAX
  #define UCHAR_MAX (SCHAR_MAX * 2U + 1U)
#else
  #define UCHAR_MAX (SCHAR_MAX * 2 + 1)
#endif

/**** CHAR_MAX & CHAR_MIN ********************************/

#undef CHAR_MAX
#undef CHAR_MIN

#ifdef __CHAR_UNSIGNED__
  #define CHAR_MAX UCHAR_MAX
  #if SCHAR_MAX == INT_MAX
    #define CHAR_MIN 0U
  #else
    #define CHAR_MIN 0
  #endif
#else
  #define CHAR_MAX SCHAR_MAX
  #define CHAR_MIN SCHAR_MIN
#endif

/**** SHRT_MAX & SHRT_MIN ********************************/

#undef SHRT_MAX
#undef SHRT_MIN

#ifdef __SHRT_MAX__
  #define SHRT_MAX __SHRT_MAX__
#else
  #define SHRT_MAX 0x7FFF
#endif

#define SHRT_MIN (-SHRT_MAX - 1)

/**** USHRT_MAX ******************************************/

#undef USHRT_MAX
#if SHRT_MAX == INT_MAX
  #define USHRT_MAX (SHRT_MAX * 2U + 1U)
#else
  #define USHRT_MAX (SHRT_MAX * 2 + 1)
#endif

/**** LONG_MAX & LONG_MIN ********************************/

#undef LONG_MAX
#undef LONG_MIN

#ifdef __LONG_MAX__
  #define LONG_MAX __LONG_MAX__
#else
  #define LONG_MAX 0x7FFFFFFFL
#endif

#define LONG_MIN (-LONG_MAX - 1L)

/**** ULONG_MAX ******************************************/

#undef ULONG_MAX
#define ULONG_MAX (LONG_MAX * 2UL + 1UL)

/**** LLONG_MAX & LLONG_MIN ******************************/

#undef LLONG_MAX
#undef LLONG_MIN

#ifdef __LONG_LONG_MAX__
  #define LLONG_MAX __LONG_LONG_MAX__
#else
  #define LLONG_MAX 0x7FFFFFFFFFFFFFFFLL
#endif

#define LLONG_MIN (-LLONG_MAX - 1LL)

/**** ULLONG_MAX *****************************************/

#undef ULLONG_MAX
#define ULLONG_MAX (LLONG_MAX * 2ULL + 1ULL)

/**** SIZE_MAX & SSIZE_MAX *******************************/

#undef SIZE_MAX
#undef SSIZE_MAX

#ifdef __SIZE_MAX__
  #define SIZE_MAX __SIZE_MAX__
#else
  #define SIZE_MAX ULONG_MAX
#endif

#define SSIZE_MAX LONG_MAX

#endif
