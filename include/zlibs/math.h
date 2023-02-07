/*
thx https://pubs.opengroup.org/onlinepubs/009695399/basedefs/math.h.html
*/

#ifndef MATH_ID
#define MATH_ID 1011

#include <type.h>
#include <math_private.h>

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

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

// functions
#define fpclassify(x) ((int (*)(float)) get_func_addr(MATH_ID, 3))(x)
#define isfinite(x) ((int (*)(float)) get_func_addr(MATH_ID, 4))(x)
#define isinf(x) ((int (*)(float)) get_func_addr(MATH_ID, 5))(x)
#define isnan(x) ((int (*)(float)) get_func_addr(MATH_ID, 6))(x)
#define isnormal(x) ((int (*)(float)) get_func_addr(MATH_ID, 7))(x)
#define signbit(x) ((int (*)(float)) get_func_addr(MATH_ID, 8))(x)
#define isgreater(x, y) ((int (*)(float, float)) get_func_addr(MATH_ID, 9))(x, y)
#define isgreaterequal(x, y) ((int (*)(float, float)) get_func_addr(MATH_ID, 10))(x, y)
#define isless(x, y) ((int (*)(float, float)) get_func_addr(MATH_ID, 11))(x, y)
#define islessequal(x, y) ((int (*)(float, float)) get_func_addr(MATH_ID, 12))(x, y)
#define islessgreater(x, y) ((int (*)(float, float)) get_func_addr(MATH_ID, 13))(x, y)
#define isunordered(x, y) ((int (*)(float, float)) get_func_addr(MATH_ID, 14))(x, y)
#define acos(x) ((double (*)(double)) get_func_addr(MATH_ID, 15))(x)
#define acosf(x) ((float (*)(float)) get_func_addr(MATH_ID, 16))(x)
#define acosh(x) ((double (*)(double)) get_func_addr(MATH_ID, 17))(x)
#define acoshf(x) ((float (*)(float)) get_func_addr(MATH_ID, 18))(x)
#define acoshl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 19))(x)
#define acosl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 20))(x)
#define asin(x) ((double (*)(double)) get_func_addr(MATH_ID, 21))(x)
#define asinf(x) ((float (*)(float)) get_func_addr(MATH_ID, 22))(x)
#define asinh(x) ((double (*)(double)) get_func_addr(MATH_ID, 23))(x)
#define asinhf(x) ((float (*)(float)) get_func_addr(MATH_ID, 24))(x)
#define asinhl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 25))(x)
#define asinl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 26))(x)
#define atan(x) ((double (*)(double)) get_func_addr(MATH_ID, 27))(x)
#define atan2(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 28))(x, y)
#define atan2f(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 29))(x, y)
#define atan2l(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 30))(x, y)
#define atanf(x) ((float (*)(float)) get_func_addr(MATH_ID, 31))(x)
#define atanh(x) ((double (*)(double)) get_func_addr(MATH_ID, 32))(x)
#define atanhf(x) ((float (*)(float)) get_func_addr(MATH_ID, 33))(x)
#define atanhl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 34))(x)
#define atanl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 35))(x)
#define cbrt(x) ((double (*)(double)) get_func_addr(MATH_ID, 36))(x)
#define cbrtf(x) ((float (*)(float)) get_func_addr(MATH_ID, 37))(x)
#define cbrtl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 38))(x)
#define ceil(x) ((double (*)(double)) get_func_addr(MATH_ID, 39))(x)
#define ceilf(x) ((float (*)(float)) get_func_addr(MATH_ID, 40))(x)
#define ceill(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 41))(x)
#define copysign(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 42))(x, y)
#define copysignf(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 43))(x, y)
#define copysignl(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 44))(x, y)
#define cos(x) ((double (*)(double)) get_func_addr(MATH_ID, 45))(x)
#define cosf(x) ((float (*)(float)) get_func_addr(MATH_ID, 46))(x)
#define cosh(x) ((double (*)(double)) get_func_addr(MATH_ID, 47))(x)
#define coshf(x) ((float (*)(float)) get_func_addr(MATH_ID, 48))(x)
#define coshl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 49))(x)
#define cosl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 50))(x)
#define erf(x) ((double (*)(double)) get_func_addr(MATH_ID, 51))(x)
#define erfc(x) ((double (*)(double)) get_func_addr(MATH_ID, 52))(x)
#define erfcf(x) ((float (*)(float)) get_func_addr(MATH_ID, 53))(x)
#define erfcl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 54))(x)
#define erff(x) ((float (*)(float)) get_func_addr(MATH_ID, 55))(x)
#define erfl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 56))(x)
#define exp(x) ((float (*)(float)) get_func_addr(MATH_ID, 57))(x)
#define exp2(x) ((double (*)(double)) get_func_addr(MATH_ID, 58))(x)
#define exp2f(x) ((float (*)(float)) get_func_addr(MATH_ID, 59))(x)
#define exp2l(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 60))(x)
#define expf(x) ((float (*)(float)) get_func_addr(MATH_ID, 61))(x)
#define expl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 62))(x)
#define expm1(x) ((double (*)(double)) get_func_addr(MATH_ID, 63))(x)
#define expm1f(x) ((float (*)(float)) get_func_addr(MATH_ID, 64))(x)
#define expm1l(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 65))(x)
#define fabs(x) ((double (*)(double)) get_func_addr(MATH_ID, 66))(x)
#define fabsf(x) ((float (*)(float)) get_func_addr(MATH_ID, 67))(x)
#define fabsl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 68))(x)
#define fdim(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 69))(x, y)
#define fdimf(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 70))(x, y)
#define fdiml(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 71))(x, y)
#define floor(x) ((double (*)(double)) get_func_addr(MATH_ID, 72))(x)
#define floorf(x) ((float (*)(float)) get_func_addr(MATH_ID, 73))(x)
#define floorl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 74))(x)
#define fma(x, y, z) ((double (*)(double, double, double)) get_func_addr(MATH_ID, 75))(x, y, z)
#define fmaf(x, y, z) ((float (*)(float, float, float)) get_func_addr(MATH_ID, 76))(x, y, z)
#define fmal(x, y, z) ((long double (*)(long double, long double, long double)) get_func_addr(MATH_ID, 77))(x, y, z)
#define fmax(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 78))(x, y)
#define fmaxf(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 79))(x, y)
#define fmaxl(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 80))(x, y)
#define fmin(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 81))(x, y)
#define fminf(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 82))(x, y)
#define fminl(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 83))(x, y)
#define fmod(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 84))(x, y)
#define fmodf(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 85))(x, y)
#define fmodl(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 86))(x, y)
#define frexp(x, y) ((double (*)(double, int *)) get_func_addr(MATH_ID, 87))(x, y)
#define frexpf(x, y) ((float (*)(float, int *)) get_func_addr(MATH_ID, 88))(x, y)
#define frexpl(x, y) ((long double (*)(long double, int *)) get_func_addr(MATH_ID, 89))(x, y)
#define hypot(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 90))(x, y)
#define hypotf(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 91))(x, y)
#define hypotl(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 92))(x, y)
#define ilogb(x) ((int (*)(double)) get_func_addr(MATH_ID, 93))(x)
#define ilogbf(x) ((int (*)(float)) get_func_addr(MATH_ID, 94))(x)
#define ilogbl(x) ((int (*)(long double)) get_func_addr(MATH_ID, 95))(x)
#define j0(x) ((double (*)(double)) get_func_addr(MATH_ID, 96))(x)
#define j1(x) ((double (*)(double)) get_func_addr(MATH_ID, 97))(x)
#define jn(x, y) ((double (*)(int, double)) get_func_addr(MATH_ID, 98))(x, y)
#define ldexp(x, y) ((double (*)(double, int)) get_func_addr(MATH_ID, 99))(x, y)
#define ldexpf(x, y) ((float (*)(float, int)) get_func_addr(MATH_ID, 100))(x, y)
#define ldexpl(x, y) ((long double (*)(long double, int)) get_func_addr(MATH_ID, 101))(x, y)
#define lgamma(x) ((double (*)(double)) get_func_addr(MATH_ID, 102))(x)
#define lgammaf(x) ((float (*)(float)) get_func_addr(MATH_ID, 103))(x)
#define lgammal(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 104))(x)
#define llrint(x) ((long long (*)(double)) get_func_addr(MATH_ID, 105))(x)
#define llrintf(x) ((long long (*)(float)) get_func_addr(MATH_ID, 106))(x)
#define llrintl(x) ((long long (*)(long double)) get_func_addr(MATH_ID, 107))(x)
#define llround(x) ((long long (*)(double)) get_func_addr(MATH_ID, 108))(x)
#define llroundf(x) ((long long (*)(float)) get_func_addr(MATH_ID, 109))(x)
#define llroundl(x) ((long long (*)(long double)) get_func_addr(MATH_ID, 110))(x)
#define log(x) ((double (*)(double)) get_func_addr(MATH_ID, 111))(x)
#define log10(x) ((double (*)(double)) get_func_addr(MATH_ID, 112))(x)
#define log10f(x) ((float (*)(float)) get_func_addr(MATH_ID, 113))(x)
#define log10l(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 114))(x)
#define log1p(x) ((double (*)(double)) get_func_addr(MATH_ID, 115))(x)
#define log1pf(x) ((float (*)(float)) get_func_addr(MATH_ID, 116))(x)
#define log1pl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 117))(x)
#define log2(x) ((double (*)(double)) get_func_addr(MATH_ID, 118))(x)
#define log2f(x) ((float (*)(float)) get_func_addr(MATH_ID, 119))(x)
#define log2l(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 120))(x)
#define logb(x) ((double (*)(double)) get_func_addr(MATH_ID, 121))(x)
#define logbf(x) ((float (*)(float)) get_func_addr(MATH_ID, 122))(x)
#define logbl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 123))(x)
#define logf(x) ((float (*)(float)) get_func_addr(MATH_ID, 124))(x)
#define logl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 125))(x)
#define lrint(x) ((long (*)(double)) get_func_addr(MATH_ID, 126))(x)
#define lrintf(x) ((long (*)(double)) get_func_addr(MATH_ID, 127))(x)
#define lrintl(x) ((long (*)(float)) get_func_addr(MATH_ID, 128))(x)
#define lround(x) ((long (*)(double)) get_func_addr(MATH_ID, 129))(x)
#define lroundf(x) ((long (*)(float)) get_func_addr(MATH_ID, 130))(x)
#define lroundl(x) ((long (*)(long double)) get_func_addr(MATH_ID, 131))(x)
#define modf(x, y) ((double (*)(double, double *)) get_func_addr(MATH_ID, 132))(x, y)
#define modff(x, y) ((float (*)(float, float *)) get_func_addr(MATH_ID, 133))(x, y)
#define modfl(x, y) ((long double (*)(long double, long double *)) get_func_addr(MATH_ID, 134))(x, y)
#define nan(x) ((double (*)(char *)) get_func_addr(MATH_ID, 135))(x)
#define nanf(x) ((float (*)(char *)) get_func_addr(MATH_ID, 136))(x)
#define nanl(x) ((long double (*)(char *)) get_func_addr(MATH_ID, 137))(x)
#define nearbyint(x) ((double (*)(double)) get_func_addr(MATH_ID, 138))(x)
#define nearbyintf(x) ((float (*)(float)) get_func_addr(MATH_ID, 139))(x)
#define nearbyintl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 140))(x)
#define nextafter(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 141))(x, y)
#define nextafterf(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 142))(x, y)
#define nextafterl(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 143))(x, y)
#define nexttoward(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 144))(x, y)
#define nexttowardf(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 145))(x, y)
#define nexttowardl(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 146))(x, y)
#define pow(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 147))(x, y)
#define powf(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 148))(x, y)
#define powl(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 149))(x, y)
#define remainder(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 150))(x, y)
#define remainderf(x, y) ((float (*)(float, float)) get_func_addr(MATH_ID, 151))(x, y)
#define remainderl(x, y) ((long double (*)(long double, long double)) get_func_addr(MATH_ID, 152))(x, y)
#define remquo(x, y, z) ((double (*)(double, double, int *)) get_func_addr(MATH_ID, 153))(x, y, z)
#define remquof(x, y, z) ((float (*)(float, float, int *)) get_func_addr(MATH_ID, 154))(x, y, z)
#define remquol(x, y, z) ((long double (*)(long double, long double, int *)) get_func_addr(MATH_ID, 155))(x, y, z)
#define rint(x) ((double (*)(double)) get_func_addr(MATH_ID, 156))(x)
#define rintf(x) ((float (*)(float)) get_func_addr(MATH_ID, 157))(x)
#define rintl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 158))(x)
#define round(x) ((double (*)(double)) get_func_addr(MATH_ID, 159))(x)
#define roundf(x) ((float (*)(float)) get_func_addr(MATH_ID, 160))(x)
#define roundl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 161))(x)
#define scalb(x, y) ((double (*)(double, double)) get_func_addr(MATH_ID, 162))(x, y)
#define scalbln(x, y) ((double (*)(double, long int)) get_func_addr(MATH_ID, 163))(x, y)
#define scalblnf(x, y) ((float (*)(float, long int)) get_func_addr(MATH_ID, 164))(x, y)
#define scalblnl(x, y) ((long double (*)(long double, long int)) get_func_addr(MATH_ID, 165))(x, y)
#define scalbn(x, y) ((double (*)(double, int)) get_func_addr(MATH_ID, 166))(x, y)
#define scalbnf(x, y) ((float (*)(float, int)) get_func_addr(MATH_ID, 167))(x, y)
#define scalbnl(x, y) ((long double (*)(long double, int)) get_func_addr(MATH_ID, 168))(x, y)
#define sin(x) ((double (*)(double)) get_func_addr(MATH_ID, 169))(x)
#define sinf(x) ((float (*)(float)) get_func_addr(MATH_ID, 170))(x)
#define sinh(x) ((double (*)(double)) get_func_addr(MATH_ID, 171))(x)
#define sinhf(x) ((float (*)(float)) get_func_addr(MATH_ID, 172))(x)
#define sinhl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 173))(x)
#define sinl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 174))(x)
#define sqrt(x) ((double (*)(double)) get_func_addr(MATH_ID, 175))(x)
#define sqrtf(x) ((float (*)(float)) get_func_addr(MATH_ID, 176))(x)
#define sqrtl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 177))(x)
#define tan(x) ((double (*)(double)) get_func_addr(MATH_ID, 178))(x)
#define tanf(x) ((float (*)(float)) get_func_addr(MATH_ID, 179))(x)
#define tanh(x) ((double (*)(double)) get_func_addr(MATH_ID, 180))(x)
#define tanhf(x) ((float (*)(float)) get_func_addr(MATH_ID, 181))(x)
#define tanhl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 182))(x)
#define tanl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 183))(x)
#define tgamma(x) ((double (*)(double)) get_func_addr(MATH_ID, 184))(x)
#define tgammaf(x) ((float (*)(float)) get_func_addr(MATH_ID, 185))(x)
#define tgammal(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 186))(x)
#define trunc(x) ((double (*)(float)) get_func_addr(MATH_ID, 187))(x)
#define truncf(x) ((float (*)(float)) get_func_addr(MATH_ID, 188))(x)
#define truncl(x) ((long double (*)(long double)) get_func_addr(MATH_ID, 189))(x)
#define y0(x) ((double (*)(double)) get_func_addr(MATH_ID, 190))(x)
#define y1(x) ((double (*)(double)) get_func_addr(MATH_ID, 191))(x)
#define yn(x, y) ((double (*)(int, double)) get_func_addr(MATH_ID, 192))(x, y)

#endif
