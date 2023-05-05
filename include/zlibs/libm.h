#ifndef LIBM_H
#define LIBM_H

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

#endif
