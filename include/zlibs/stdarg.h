#if !defined(__FSTD_HDRS_STDARG_H) || defined(__FSTD_HDRS_CXX_WRAP)

#if !defined(__FSTD_HDRS_CXX_WRAP)
#define __FSTD_HDRS_STDARG_H 1
#endif

typedef __builtin_va_list va_list;

#undef va_start
#define va_start(v, l) __builtin_va_start(v, l)

#undef va_end
#define va_end(v) __builtin_va_end(v)

#undef va_arg
#define va_arg(v, l) __builtin_va_arg(v, l)

#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#  undef va_copy
#  define va_copy(d, s) __builtin_va_copy(d, s)
#endif

#endif
