/*****************************************************************************\
|   === stdint.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the (32bit) stdint.h header file            .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _STDINT_H
#define _STDINT_H

/**** int_t & uint_t *************************************/

#ifdef __INT8_TYPE__
  typedef __INT8_TYPE__  int8_t;
  typedef __INT16_TYPE__ int16_t;
  typedef __INT32_TYPE__ int32_t;
  typedef __INT64_TYPE__ int64_t;

  typedef __UINT8_TYPE__  uint8_t;
  typedef __UINT16_TYPE__ uint16_t;
  typedef __UINT32_TYPE__ uint32_t;
  typedef __UINT64_TYPE__ uint64_t;
#else
  typedef signed char int8_t;
  typedef short       int16_t;
  typedef int         int32_t;
  typedef long long   int64_t;

  typedef unsigned char      uint8_t;
  typedef unsigned short     uint16_t;
  typedef unsigned int       uint32_t;
  typedef unsigned long long uint64_t;
#endif

/**** int_least_t & uint_least_t *************************/

#ifdef __INT_LEAST8_TYPE__
  typedef __INT_LEAST8_TYPE__  int_least8_t;
  typedef __INT_LEAST16_TYPE__ int_least16_t;
  typedef __INT_LEAST32_TYPE__ int_least32_t;
  typedef __INT_LEAST64_TYPE__ int_least64_t;

  typedef __UINT_LEAST8_TYPE__  uint_least8_t;
  typedef __UINT_LEAST16_TYPE__ uint_least16_t;
  typedef __UINT_LEAST32_TYPE__ uint_least32_t;
  typedef __UINT_LEAST64_TYPE__ uint_least64_t;
#else
  typedef int8_t  int_least8_t;
  typedef int16_t int_least16_t;
  typedef int32_t int_least32_t;
  typedef int64_t int_least64_t;

  typedef uint8_t  uint_least8_t;
  typedef uint16_t uint_least16_t;
  typedef uint32_t uint_least32_t;
  typedef uint64_t uint_least64_t;
#endif

/**** int_fast_t & uint_fast_t ***************************/

#ifdef __INT_FAST8_TYPE__
  typedef __INT_FAST8_TYPE__  int_fast8_t;
  typedef __INT_FAST16_TYPE__ int_fast16_t;
  typedef __INT_FAST32_TYPE__ int_fast32_t;
  typedef __INT_FAST64_TYPE__ int_fast64_t;

  typedef __UINT_FAST8_TYPE__  uint_fast8_t;
  typedef __UINT_FAST16_TYPE__ uint_fast16_t;
  typedef __UINT_FAST32_TYPE__ uint_fast32_t;
  typedef __UINT_FAST64_TYPE__ uint_fast64_t;
#else
  typedef int_least8_t  int_fast8_t;
  typedef int_least16_t int_fast16_t;
  typedef int_least32_t int_fast32_t;
  typedef int_least64_t int_fast64_t;

  typedef uint_least8_t  uint_fast8_t;
  typedef uint_least16_t uint_fast16_t;
  typedef uint_least32_t uint_fast32_t;
  typedef uint_least64_t uint_fast64_t;
#endif

/**** intptr_t & uintptr_t *******************************/

#ifdef __INTPTR_TYPE__
  typedef __UINTPTR_TYPE__ uintptr_t;
  typedef __INTPTR_TYPE__  intptr_t;
#else
  typedef unsigned int uintptr_t;
  typedef int          intptr_t;
#endif

/**** intmax_t & uintmax_t *******************************/

#ifdef __INTMAX_TYPE__
  typedef __INTMAX_TYPE__  intmax_t;
  typedef __UINTMAX_TYPE__ uintmax_t;
#else
  typedef long long int          intmax_t;
  typedef unsigned long long int uintmax_t;
#endif

/**** INT_C & UINT_C & INTMAX_C & UINTMAX_C **************/

#undef INT8_C
#undef INT16_C
#undef INT32_C
#undef INT64_C
#undef UINT8_C
#undef UINT16_C
#undef UINT32_C
#undef UINT64_C

#undef INTMAX_C
#undef UINTMAX_C

#if defined(__clang__)
  #define INT8_C(x)  x ## __INT8_C_SUFFIX__
  #define INT16_C(x) x ## __INT16_C_SUFFIX__
  #define INT32_C(x) x ## __INT32_C_SUFFIX__
  #define INT64_C(x) x ## __INT64_C_SUFFIX__

  #define UINT8_C(x)  x ## __UINT8_C_SUFFIX__
  #define UINT16_C(x) x ## __UINT16_C_SUFFIX__
  #define UINT32_C(x) x ## __UINT32_C_SUFFIX__
  #define UINT64_C(x) x ## __UINT64_C_SUFFIX__

  #define INTMAX_C(x)  x ## __INTMAX_C_SUFFIX__
  #define UINTMAX_C(x) x ## __UINTMAX_C_SUFFIX__
#elif defined(__GNUC__)
  #define INT8_C(x)  __INT8_C(x)
  #define INT16_C(x) __INT16_C(x)
  #define INT32_C(x) __INT32_C(x)
  #define INT64_C(x) __INT64_C(x)

  #define UINT8_C(x)  __UINT8_C(x)
  #define UINT16_C(x) __UINT16_C(x)
  #define UINT32_C(x) __UINT32_C(x)
  #define UINT64_C(x) __UINT64_C(x)

  #define INTMAX_C(x)  __INTMAX_C(x)
  #define UINTMAX_C(x) __UINTMAX_C(x)
#else
  #define INT8_C(c)  c
  #define INT16_C(c) c
  #define INT32_C(c) c
  #define INT64_C(c) c ## LL

  #define UINT8_C(c)  c
  #define UINT16_C(c) c
  #define UINT32_C(c) c ## U
  #define UINT64_C(c) c ## ULL

  #define INTMAX_C(c)  c ## LL
  #define UINTMAX_C(c) c ## ULL
#endif

/**** UINT_MAX & INT_MAX & INT_MIN ***********************/

#undef UINT8_MAX
#undef UINT16_MAX
#undef UINT32_MAX
#undef UINT64_MAX

#ifdef __UINT8_MAX__
  #define UINT8_MAX  __UINT8_MAX__
  #define UINT16_MAX __UINT16_MAX__
  #define UINT32_MAX __UINT32_MAX__
  #define UINT64_MAX __UINT64_MAX__
#else
  #define UINT8_MAX  0xff
  #define UINT16_MAX 0xffff
  #define UINT32_MAX 0xffffffff
  #define UINT64_MAX 0xffffffffffffffff
#endif

#undef INT8_MAX
#undef INT16_MAX
#undef INT32_MAX
#undef INT64_MAX

#ifdef __INT8_MAX__
  #define INT8_MAX  __INT8_MAX__
  #define INT16_MAX __INT16_MAX__
  #define INT32_MAX __INT32_MAX__
  #define INT64_MAX __INT64_MAX__
#else
  #define INT8_MAX  0x7f
  #define INT16_MAX 0x7fff
  #define INT32_MAX 0x7fffffff
  #define INT64_MAX 0x7fffffffffffffff
#endif

#undef INT8_MIN
#undef INT16_MIN
#undef INT32_MIN
#undef INT64_MIN

#define INT8_MIN  (-INT8_MAX  - 1)
#define INT16_MIN (-INT16_MAX - 1)
#define INT32_MIN (-INT32_MAX - 1)
#define INT64_MIN (-INT64_MAX - 1)

/**** UINT_LEAST_MAX & INT_LEAST_MAX & INT_LEAST_MIN *****/

#undef UINT_LEAST8_MAX
#undef UINT_LEAST16_MAX
#undef UINT_LEAST32_MAX
#undef UINT_LEAST64_MAX

#ifdef __UINT_LEAST8_MAX__
  #define UINT_LEAST8_MAX  __UINT_LEAST8_MAX__
  #define UINT_LEAST16_MAX __UINT_LEAST16_MAX__
  #define UINT_LEAST32_MAX __UINT_LEAST32_MAX__
  #define UINT_LEAST64_MAX __UINT_LEAST64_MAX__
#else
  #define UINT_LEAST8_MAX  UINT8_MAX
  #define UINT_LEAST16_MAX UINT16_MAX
  #define UINT_LEAST32_MAX UINT32_MAX
  #define UINT_LEAST64_MAX UINT64_MAX
#endif

#undef INT_LEAST8_MAX
#undef INT_LEAST16_MAX
#undef INT_LEAST32_MAX
#undef INT_LEAST64_MAX

#ifdef __INT_LEAST8_MAX__
  #define INT_LEAST8_MAX  __INT_LEAST8_MAX__
  #define INT_LEAST16_MAX __INT_LEAST16_MAX__
  #define INT_LEAST32_MAX __INT_LEAST32_MAX__
  #define INT_LEAST64_MAX __INT_LEAST64_MAX__
#else
  #define INT_LEAST8_MAX  INT8_MAX
  #define INT_LEAST16_MAX INT16_MAX
  #define INT_LEAST32_MAX INT32_MAX
  #define INT_LEAST64_MAX INT64_MAX
#endif

#undef INT_LEAST8_MIN
#undef INT_LEAST16_MIN
#undef INT_LEAST32_MIN
#undef INT_LEAST64_MIN

#define INT_LEAST8_MIN  (-INT_LEAST8_MAX  - 1)
#define INT_LEAST16_MIN (-INT_LEAST16_MAX - 1)
#define INT_LEAST32_MIN (-INT_LEAST32_MAX - 1)
#define INT_LEAST64_MIN (-INT_LEAST64_MAX - 1)

/**** UINT_FAST_MAX & INT_FAST_MAX & INT_FAST_MIN ********/

#undef UINT_FAST8_MAX
#undef UINT_FAST16_MAX
#undef UINT_FAST32_MAX
#undef UINT_FAST64_MAX

#ifdef __UINT_FAST8_MAX__
  #define UINT_FAST8_MAX  __UINT_FAST8_MAX__
  #define UINT_FAST16_MAX __UINT_FAST16_MAX__
  #define UINT_FAST32_MAX __UINT_FAST32_MAX__
  #define UINT_FAST64_MAX __UINT_FAST64_MAX__
#else
  #define UINT_FAST8_MAX  UINT8_MAX
  #define UINT_FAST16_MAX UINT16_MAX
  #define UINT_FAST32_MAX UINT32_MAX
  #define UINT_FAST64_MAX UINT64_MAX
#endif

#undef INT_FAST8_MAX
#undef INT_FAST16_MAX
#undef INT_FAST32_MAX
#undef INT_FAST64_MAX

#ifdef __INT_FAST8_MAX__
  #define INT_FAST8_MAX  __INT_FAST8_MAX__
  #define INT_FAST16_MAX __INT_FAST16_MAX__
  #define INT_FAST32_MAX __INT_FAST32_MAX__
  #define INT_FAST64_MAX __INT_FAST64_MAX__
#else
  #define INT_FAST8_MAX  INT8_MAX
  #define INT_FAST16_MAX INT16_MAX
  #define INT_FAST32_MAX INT32_MAX
  #define INT_FAST64_MAX INT64_MAX
#endif

#undef INT_FAST8_MIN
#undef INT_FAST16_MIN
#undef INT_FAST32_MIN
#undef INT_FAST64_MIN

#define INT_FAST8_MIN  (-INT_FAST8_MAX  - 1)
#define INT_FAST16_MIN (-INT_FAST16_MAX - 1)
#define INT_FAST32_MIN (-INT_FAST32_MAX - 1)
#define INT_FAST64_MIN (-INT_FAST64_MAX - 1)

/**** UINTPTR_MAX & INTPTR_MAX & INTPTR_MIN **************/

#undef UINTPTR_MAX
#undef INTPTR_MAX
#undef INTPTR_MIN

#ifdef __UINTPTR_MAX__
  #define UINTPTR_MAX __UINTPTR_MAX__
#else
  #define UINTPTR_MAX UINT32_MAX
#endif

#ifdef __INTPTR_MAX__
  #define INTPTR_MAX __INTPTR_MAX__
#else
  #define INTPTR_MAX INT32_MAX
#endif

#define INTPTR_MIN (-INTPTR_MAX - 1)

/**** INTMAX_MAX & UINTMAX_MAX & INTMAX_MIN **************/

#undef UINTMAX_MAX
#undef INTMAX_MAX
#undef INTMAX_MIN

#ifdef __UINTMAX_MAX__
  #define UINTMAX_MAX __UINTMAX_MAX__
#else
  #define UINTMAX_MAX UINT64_MAX
#endif

#ifdef __INTMAX_MAX__
  #define INTMAX_MAX __INTMAX_MAX__
#else
  #define INTMAX_MAX INT64_MAX
#endif

#define INTMAX_MIN (-INTMAX_MAX - 1)

/**** PTRDIFF_MAX & PTRDIFF_MIN *************************/

#undef PTRDIFF_MAX
#undef PTRDIFF_MIN

#ifdef __PTRDIFF_MAX__
  #define PTRDIFF_MAX __PTRDIFF_MAX__
#else
  #define PTRDIFF_MAX INT32_MAX
#endif

#define PTRDIFF_MIN (-PTRDIFF_MAX - 1)

/**** SIG_ATOMIC_MAX & SIG_ATOMIC_MIN *******************/

#undef SIG_ATOMIC_MAX
#undef SIG_ATOMIC_MIN

#ifdef __SIG_ATOMIC_MAX__
  #define SIG_ATOMIC_MAX __SIG_ATOMIC_MAX__
#else
  #define SIG_ATOMIC_MAX INT32_MAX
#endif

#define SIG_ATOMIC_MIN (-SIG_ATOMIC_MAX - 1)

/**** SIZE_MAX ******************************************/

#undef SIZE_MAX

#ifdef __SIZE_MAX__
  #define SIZE_MAX __SIZE_MAX__
#else
  #define SIZE_MAX UINT32_MAX
#endif

/**** WCHAR_MAX & WCHAR_MIN ******************************/

#undef WCHAR_MAX
#undef WCHAR_MIN

#ifdef __WCHAR_MAX__
  #define WCHAR_MAX __WCHAR_MAX__
#else
  #define WCHAR_MAX UINT32_MAX
#endif

#define WCHAR_MIN (-WCHAR_MAX - 1)

/**** WINT_MAX & WINT_MIN *******************************/

#undef WINT_MAX
#undef WINT_MIN

#ifdef __WINT_MAX__
  #define WINT_MAX __WINT_MAX__
#else
  #define WINT_MAX UINT32_MAX
#endif

#define WINT_MIN (-WINT_MAX - 1)

#endif
