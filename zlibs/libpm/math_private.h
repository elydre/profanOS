/*****************************************************************************\
|   === math_private.h : 2024 ===                                             |
|                                                                             |
|    Private header file for profanOS mini libm                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original code from sun microsystems (see below)               `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _MATH_PRIVATE_H
#define _MATH_PRIVATE_H

#include <stdint.h>

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

typedef union {
  double value;
  struct {
    uint32_t lsw;
    uint32_t msw;
  } parts;
  uint64_t word;
} ieee_double_shape_type;

typedef union {
    float value;
    uint32_t word;
} ieee_float_shape_type;

/* The original fdlibm code used statements like:
    n0 = ((*(int*)&one)>>29)^1;        * index of high word *
    ix0 = *(n0+(int*)&x);              * high word of x *
    ix1 = *((1-n0)+(int*)&x);          * low word of x *
   to dig two 32 bit words out of the 64 bit IEEE floating point
   value.  That is non-ANSI, and, moreover, the gcc instruction
   scheduler gets it wrong.  We instead use the following macros.
   Unlike the original code, we determine the endianness at compile
   time, not at run time; I don't see much benefit to selecting
   endianness at run time.  */

// Get two 32 bit ints from a double.
#define EXTRACT_WORDS(ix0,ix1,d)    \
do {                                \
  ieee_double_shape_type ew_u;      \
  ew_u.value = (d);                 \
  (ix0) = ew_u.parts.msw;           \
  (ix1) = ew_u.parts.lsw;           \
} while (0)

// Get the more significant 32 bit int from a double.
#ifndef GET_HIGH_WORD
# define GET_HIGH_WORD(i,d)         \
do {                                \
  ieee_double_shape_type gh_u;      \
  gh_u.value = (d);                 \
  (i) = gh_u.parts.msw;             \
} while (0)
#endif

// Get the less significant 32 bit int from a double.
#ifndef GET_LOW_WORD
# define GET_LOW_WORD(i,d)          \
do {                                \
  ieee_double_shape_type gl_u;      \
  gl_u.value = (d);                 \
  (i) = gl_u.parts.lsw;             \
} while (0)
#endif

// Get all in one, efficient on 64-bit machines.
#ifndef EXTRACT_WORDS64
# define EXTRACT_WORDS64(i,d)       \
do {                                \
  ieee_double_shape_type gh_u;      \
  gh_u.value = (d);                 \
  (i) = gh_u.word;                  \
} while (0)
#endif

// Set a double from two 32 bit ints.
#ifndef INSERT_WORDS
# define INSERT_WORDS(d,ix0,ix1)    \
do {                                \
  ieee_double_shape_type iw_u;      \
  iw_u.parts.msw = (ix0);           \
  iw_u.parts.lsw = (ix1);           \
  (d) = iw_u.value;                 \
} while (0)
#endif

// Get all in one, efficient on 64-bit machines.
#ifndef INSERT_WORDS64
# define INSERT_WORDS64(d,i)        \
do {                                \
  ieee_double_shape_type iw_u;      \
  iw_u.word = (i);                  \
  (d) = iw_u.value;                 \
} while (0)
#endif

// Set the more significant 32 bits of a double from an int.
#ifndef SET_HIGH_WORD
#define SET_HIGH_WORD(d,v)          \
do {                                \
  ieee_double_shape_type sh_u;      \
  sh_u.value = (d);                 \
  sh_u.parts.msw = (v);             \
  (d) = sh_u.value;                 \
} while (0)
#endif

// Set the less significant 32 bits of a double from an int.
#ifndef SET_LOW_WORD
# define SET_LOW_WORD(d,v)          \
do {                                \
  ieee_double_shape_type sl_u;      \
  sl_u.value = (d);                 \
  sl_u.parts.lsw = (v);             \
  (d) = sl_u.value;                 \
} while (0)
#endif

// Get a 32 bit int from a float.
#ifndef GET_FLOAT_WORD
# define GET_FLOAT_WORD(i,d)        \
do {                                \
    ieee_float_shape_type gf_u;     \
    gf_u.value = (d);               \
    (i) = gf_u.word;                \
} while (0)
#endif

// Set a float from a 32 bit int.
#ifndef SET_FLOAT_WORD
# define SET_FLOAT_WORD(d,i)        \
do {                                \
    ieee_float_shape_type sf_u;     \
    sf_u.word = (i);                \
    (d) = sf_u.value;               \
} while (0)
#endif

#endif
