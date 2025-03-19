/*****************************************************************************\
|   === math.h : 2024 ===                                                     |
|                                                                             |
|    Header for profanOS mini libm                                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_MATH_H
#define _PROFAN_MATH_H

#define M_E        2.7182818284590452354
#define M_LOG2E    1.4426950408889634074
#define M_LOG10E   0.43429448190325182765
#define M_LN2      0.69314718055994530942
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.78539816339744830962
#define M_1_PI     0.31830988618379067154
#define M_2_PI     0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.70710678118654752440

#define MAXFLOAT   3.40282346638528860e+38F

// libm header
#define FORCE_EVAL(x) do {                      \
    if (sizeof(x) == sizeof(float)) {           \
        volatile float __x;                     \
        __x = (x);                              \
    } else if (sizeof(x) == sizeof(double)) {   \
        volatile double __x;                    \
        __x = (x);                              \
    } else {                                    \
        volatile long double __x;               \
        __x = (x);                              \
    }                                           \
} while(0)

#define asuint(f) ((union{float _f; uint32_t _i;}){f})._i
#define asfloat(i) ((union{uint32_t _i; float _f;}){i})._f
#define asuint64(f) ((union{double _f; uint64_t _i;}){f})._i
#define asdouble(i) ((union{uint64_t _i; double _f;}){i})._f

float  acosf(float x);
float  cosf(float x);
float  fabsf(float x);
double floor(double x);
double log10(double x);
float  powf(float x, float y);
float  sinf(float x);
double sqrt(double x);
float  sqrtf(float x);
float  tanf(float x);
double trunc(double x);
float  truncf(float x);

#endif
