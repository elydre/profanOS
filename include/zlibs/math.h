#ifndef MATH_H
#define MATH_H

#include <type.h>

extern int signgam;

#define float_t float
#define double_t double

#define M_E 2.7182818284590452354
#define M_LOG2E 1.4426950408889634074
#define M_LOG10E 0.43429448190325182765
#define M_LN2 0.69314718055994530942
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.78539816339744830962
#define M_1_PI 0.31830988618379067154
#define M_2_PI 0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2 1.41421356237309504880
#define M_SQRT1_2 0.70710678118654752440

#define MAXFLOAT 3.40282346638528860e+38F
#define HUGE_VAL 1e5000
#define HUGE_VALF 1e50
#define HUGE_VALL 1e5000L
#define INFINITY HUGE_VALF
#define NAN HUGE_VALF

#define FP_INFINITE 1
#define FP_NAN 2
#define FP_NORMAL 3
#define FP_SUBNORMAL 4
#define FP_ZERO 5

#define FP_FAST_FMA 1
#define FP_FAST_FMAF 1
#define FP_FAST_FMAL 1

#define FP_ILOGB0 (-2147483647 - 1)
#define FP_ILOGBNAN (-2147483647 - 1)

#define MATH_ERRNO 1
#define MATH_ERREXCEPT 2

#define math_errhandling MATH_ERRNO | MATH_ERREXCEPT

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

double acos(double x);
float acosf(float x);
float cosf(float x);
float expm1f(float x);
double floor(double x);
float floorf(float a);
float frexpf(float x, int *e);
double log10(double x);
float powf(float x, float y);
double scalbn(double x, int n);
float sinf(float x);
float sinhf(float x);
long double sinhl(long double x);
long double sinl(long double x);
double sqrt(double x);
float sqrtf(float x);
float tanf(float x);
float tanhf(float x);
long double tanhl(long double x);
long double tanl(long double x);
double tgamma(double x);
float tgammaf(float x);
long double tgammal(long double x);
double trunc(double x);
float truncf(float x);
long double truncl(long double x);

float fabsf(float x);
float asinf(float x);
float atan2f(float y, float x);
float fmodf(float x, float y);
float ceilf(float x);
float log2f(float x);
float log10f(float x);
float logf(float x);
float ldexpf(float x, int exp);
float expf(float x);


#endif
