/*****************************************************************************\
|   === float.h : 2024 ===                                                    |
|                                                                             |
|    Implementation of the (32bit) float.h header file             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _FLOAT_H
#define _FLOAT_H

/**** FLT_ROUNDS *****************************************/

#undef FLT_ROUNDS
#define FLT_ROUNDS 1

/**** FLT_RADIX ******************************************/

#undef FLT_RADIX

#ifdef __FLT_RADIX__
  #define FLT_RADIX __FLT_RADIX__
#else
  #define FLT_RADIX 2
#endif

/**** FLT_MANT_DIG ***************************************/

#undef FLT_MANT_DIG

#ifdef __FLT_MANT_DIG__
  #define FLT_MANT_DIG __FLT_MANT_DIG__
#else
  #define FLT_MANT_DIG 24
#endif

/**** DBL_MANT_DIG ***************************************/

#undef DBL_MANT_DIG

#ifdef __DBL_MANT_DIG__
  #define DBL_MANT_DIG __DBL_MANT_DIG__
#else
  #define DBL_MANT_DIG 53
#endif

/**** LDBL_MANT_DIG **************************************/

#undef LDBL_MANT_DIG

#ifdef __LDBL_MANT_DIG__
  #define LDBL_MANT_DIG __LDBL_MANT_DIG__
#else
  #define LDBL_MANT_DIG 64
#endif

/**** DECIMAL_DIG ****************************************/

#undef DECIMAL_DIG

#ifdef __DECIMAL_DIG__
  #define DECIMAL_DIG __DECIMAL_DIG__
#else
  #define DECIMAL_DIG 21
#endif

/**** FLT_DECIMAL_DIG ************************************/

#undef FLT_DECIMAL_DIG

#ifdef __FLT_DECIMAL_DIG__
  #define FLT_DECIMAL_DIG __FLT_DECIMAL_DIG__
#else
  #define FLT_DECIMAL_DIG 9
#endif

/**** DBL_DECIMAL_DIG ************************************/

#undef DBL_DECIMAL_DIG

#ifdef __DBL_DECIMAL_DIG__
  #define DBL_DECIMAL_DIG __DBL_DECIMAL_DIG__
#else
  #define DBL_DECIMAL_DIG 17
#endif

/**** LDBL_DECIMAL_DIG ***********************************/

#undef LDBL_DECIMAL_DIG

#ifdef __LDBL_DECIMAL_DIG__
  #define LDBL_DECIMAL_DIG __LDBL_DECIMAL_DIG__
#else
  #define LDBL_DECIMAL_DIG 18
#endif

/**** FLT_EVAL_METHOD ************************************/

#undef FLT_EVAL_METHOD

#ifdef __FLT_EVAL_METHOD__
  #define FLT_EVAL_METHOD __FLT_EVAL_METHOD__
#else
  #define FLT_EVAL_METHOD 0
#endif

/**** FLT_DIG ********************************************/

#undef FLT_DIG

#ifdef __FLT_DIG__
  #define FLT_DIG __FLT_DIG__
#else
  #define FLT_DIG 6
#endif

/**** DBL_DIG ********************************************/

#undef DBL_DIG

#ifdef __DBL_DIG__
  #define DBL_DIG __DBL_DIG__
#else
  #define DBL_DIG 15
#endif

/**** LDBL_DIG *******************************************/

#undef LDBL_DIG

#ifdef __LDBL_DIG__
  #define LDBL_DIG __LDBL_DIG__
#else
  #define LDBL_DIG 18
#endif

/**** FLT_MIN_EXP ****************************************/

#undef FLT_MIN_EXP

#ifdef __FLT_MIN_EXP__
  #define FLT_MIN_EXP __FLT_MIN_EXP__
#else
  #define FLT_MIN_EXP (-125)
#endif

/**** DBL_MIN_EXP ****************************************/

#undef DBL_MIN_EXP

#ifdef __DBL_MIN_EXP__
  #define DBL_MIN_EXP __DBL_MIN_EXP__
#else
  #define DBL_MIN_EXP (-1021)
#endif

/**** LDBL_MIN_EXP ***************************************/

#undef LDBL_MIN_EXP

#ifdef __LDBL_MIN_EXP__
  #define LDBL_MIN_EXP __LDBL_MIN_EXP__
#else
  #define LDBL_MIN_EXP (-16381)
#endif

/**** FLT_MIN_10_EXP *************************************/

#undef FLT_MIN_10_EXP

#ifdef __FLT_MIN_10_EXP__
  #define FLT_MIN_10_EXP __FLT_MIN_10_EXP__
#else
  #define FLT_MIN_10_EXP -37
#endif

/**** DBL_MIN_10_EXP *************************************/

#undef DBL_MIN_10_EXP

#ifdef __DBL_MIN_10_EXP__
  #define DBL_MIN_10_EXP __DBL_MIN_10_EXP__
#else
  #define DBL_MIN_10_EXP -307
#endif

/**** LDBL_MIN_10_EXP ************************************/

#undef LDBL_MIN_10_EXP

#ifdef __LDBL_MIN_10_EXP__
  #define LDBL_MIN_10_EXP __LDBL_MIN_10_EXP__
#else
  #define LDBL_MIN_10_EXP -4931
#endif

/**** FLT_MAX_EXP ****************************************/

#undef FLT_MAX_EXP

#ifdef __FLT_MAX_EXP__
  #define FLT_MAX_EXP __FLT_MAX_EXP__
#else
  #define FLT_MAX_EXP 128
#endif

/**** DBL_MAX_EXP ****************************************/

#undef DBL_MAX_EXP

#ifdef __DBL_MAX_EXP__
  #define DBL_MAX_EXP __DBL_MAX_EXP__
#else
  #define DBL_MAX_EXP 1024
#endif

/**** LDBL_MAX_EXP ***************************************/

#undef LDBL_MAX_EXP

#ifdef __LDBL_MAX_EXP__
  #define LDBL_MAX_EXP __LDBL_MAX_EXP__
#else
  #define LDBL_MAX_EXP 16384
#endif

/**** FLT_MAX_10_EXP *************************************/

#undef FLT_MAX_10_EXP

#ifdef __FLT_MAX_10_EXP__
  #define FLT_MAX_10_EXP __FLT_MAX_10_EXP__
#else
  #define FLT_MAX_10_EXP 38
#endif

/**** DBL_MAX_10_EXP *************************************/

#undef DBL_MAX_10_EXP
#ifdef __DBL_MAX_10_EXP__
  #define DBL_MAX_10_EXP __DBL_MAX_10_EXP__
#else
  #define DBL_MAX_10_EXP 308
#endif

/**** LDBL_MAX_10_EXP ************************************/

#undef LDBL_MAX_10_EXP

#ifdef __LDBL_MAX_10_EXP__
  #define LDBL_MAX_10_EXP __LDBL_MAX_10_EXP__
#else
  #define LDBL_MAX_10_EXP 4932
#endif

/**** FLT_MAX ********************************************/

#undef FLT_MAX

#ifdef __FLT_MAX__
  #define FLT_MAX __FLT_MAX__
#else
  #define FLT_MAX 3.402823466e+38F
#endif

/***** DBL_MAX *******************************************/

#undef DBL_MAX

#ifdef __DBL_MAX__
  #define DBL_MAX __DBL_MAX__
#else
  #define DBL_MAX 1.7976931348623157e+308
#endif

/**** LDBL_MAX *******************************************/

#undef LDBL_MAX

#ifdef __LDBL_MAX__
  #define LDBL_MAX __LDBL_MAX__
#else
  #define LDBL_MAX 1.189731495357231765e+4932L
#endif

/**** FLT_EPSILON ****************************************/

#undef FLT_EPSILON

#ifdef __FLT_EPSILON__
  #define FLT_EPSILON __FLT_EPSILON__
#else
  #define FLT_EPSILON 1.192092896e-07F
#endif

/**** DBL_EPSILON ****************************************/

#undef DBL_EPSILON

#ifdef __DBL_EPSILON__
  #define DBL_EPSILON __DBL_EPSILON__
#else
  #define DBL_EPSILON 2.2204460492503131e-16
#endif

/**** LDBL_EPSILON ***************************************/

#undef LDBL_EPSILON

#ifdef __LDBL_EPSILON__
  #define LDBL_EPSILON __LDBL_EPSILON__
#else
  #define LDBL_EPSILON 1.084202172485504434e-19L
#endif

/**** FLT_MIN ********************************************/

#undef FLT_MIN

#ifdef __FLT_MIN__
  #define FLT_MIN __FLT_MIN__
#else
  #define FLT_MIN 1.175494351e-38F
#endif

/**** DBL_MIN ********************************************/

#undef DBL_MIN
#ifdef __DBL_MIN__
  #define DBL_MIN __DBL_MIN__
#else
  #define DBL_MIN 2.2250738585072014e-308
#endif

/**** LDBL_MIN *******************************************/

#undef LDBL_MIN

#ifdef __LDBL_MIN__
  #define LDBL_MIN __LDBL_MIN__
#else
  #define LDBL_MIN 3.3621031431120935063e-4932L
#endif

/**** FLT_TRUE_MIN ***************************************/

#undef FLT_TRUE_MIN

#ifdef __FLT_DENORM_MIN__
  #define FLT_TRUE_MIN __FLT_DENORM_MIN__
#else
  #define FLT_TRUE_MIN 1.401298464e-45F
#endif

/**** DBL_TRUE_MIN ***************************************/

#undef DBL_TRUE_MIN

#ifdef __DBL_DENORM_MIN__
  #define DBL_TRUE_MIN __DBL_DENORM_MIN__
#else
  #define DBL_TRUE_MIN 4.9406564584124654e-324
#endif

/**** LDBL_TRUE_MIN **************************************/

#undef LDBL_TRUE_MIN

#ifdef __LDBL_DENORM_MIN__
  #define LDBL_TRUE_MIN __LDBL_DENORM_MIN__
#else
  #define LDBL_TRUE_MIN 3.6451995318824746025e-4951L
#endif

/**** FLT_HAS_SUBNORM ************************************/

#undef FLT_HAS_SUBNORM

#ifdef __FLT_HAS_DENORM__
  #define FLT_HAS_SUBNORM __FLT_HAS_DENORM__
#else
  #define FLT_HAS_SUBNORM 1
#endif

/**** DBL_HAS_SUBNORM ************************************/

#undef DBL_HAS_SUBNORM

#ifdef __DBL_HAS_DENORM__
  #define DBL_HAS_SUBNORM __DBL_HAS_DENORM__
#else
  #define DBL_HAS_SUBNORM 1
#endif

/**** LDBL_HAS_SUBNORM ***********************************/

#undef LDBL_HAS_SUBNORM

#ifdef __LDBL_HAS_DENORM__
  #define LDBL_HAS_SUBNORM __LDBL_HAS_DENORM__
#else
  #define LDBL_HAS_SUBNORM 1
#endif

#endif
