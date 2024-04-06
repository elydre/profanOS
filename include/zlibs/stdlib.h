#ifndef STDLIB_ID
#define STDLIB_ID 1007

#include <profan/type.h>
#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define RAND_MAX 0x7fffffff

#define calloc(nmemb, lsize) calloc_func(nmemb, lsize, 0)
#define malloc(size) malloc_func(size, 0)
#define realloc(mem, new_size) realloc_func(mem, new_size, 0)

#define calloc_ask(nmemb, lsize) calloc_func(nmemb, lsize, 1)
#define malloc_ask(size) malloc_func(size, 1)
#define realloc_ask(mem, new_size) realloc_func(mem, new_size, 1)

void init_environ_ptr(char **env);
char **get_environ_ptr(void);
void *calloc_func(uint32_t nmemb, uint32_t lsize, int as_kernel);
void free(void *mem);
void *malloc_func(uint32_t size, int as_kernel);
void *realloc_func(void *mem, uint32_t new_size, int as_kernel);
long int a64l(const char *string);
void abort(void);
int abs(int j);
void atexit(void (*func)());
double atof(const char *s);
int atoi(const char *nptr);
long atol(const char *nptr);
long long atoll(const char *nptr);
void *bsearch(const void *key, const void *base, size_t high, size_t size, int (*compar)(const void *, const void *));
char *canonicalize_file_name(const char *name);
div_t div(int numer, int denom);
double drand48(void);
int drand48_r(struct drand48_data *buffer, double *result);
int __drand48_iterate(unsigned short int xsubi[3], struct drand48_data *buffer);
double erand48(unsigned short int xsubi[3]);
int erand48_r(unsigned short int xsubi[3], struct drand48_data *buffer, double *result);
void exit(int rv);
char *gcvt(double number, int ndigit, char *buf);
char *getenv(const char *var);
int getpt(void);
long int jrand48(unsigned short int xsubi[3]);
int jrand48_r(unsigned short int xsubi[3], struct drand48_data *buffer, long int *result);
char *l64a(long int n);
long int labs(long int j);
void lcong48(unsigned short int param[7]);
ldiv_t ldiv(long int numer, long int denom);
long long int llabs(long long int j);
lldiv_t lldiv(long long int numer, long long int denom);
long int lrand48(void);
int lrand48_r(struct drand48_data *buffer, long int *result);
int mblen(const char *s, size_t n);
size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
int mbtowc(wchar_t *pwc, const char *s, size_t n);
char *mkdtemp(char *template);
int mkostemp(char *template, int flags);
int mkostemp64(char *template, int flags);
int mkostemps(char *template, int suffixlen, int flags);
int mkostemps64(char *template, int suffixlen, int flags);
int mkstemp(char *template);
int mkstemp64(char *template);
int mkstemps(char *template, int suffixlen);
int mkstemps64(char *template, int suffixlen);
char *mktemp(char *template);
long int mrand48(void);
int mrand48_r(struct drand48_data *buffer, long int *result);
long int nrand48(unsigned short int xsubi[3]);
int nrand48_r(unsigned short int xsubi[3], struct drand48_data *buffer, long int *result);
int on_exit(oefuncp func, void *arg);
int posix_memalign(void **memptr, size_t alignment, size_t size);
char *ptsname (int fd);
void qsort(void *base, size_t nel, size_t width, __compar_fn_t comp);
void qsort_r(void *base, size_t nel, size_t width, __compar_d_fn_t comp, void *arg);
int rand(void);
int rand_r(unsigned int *seed);
long int random(void);
int random_r(struct random_data *buf, int32_t *result);
char *realpath(const char *path, char *got_path);
int rpmatch(const char *__response);
char *secure_getenv(const char *name);
unsigned short int *seed48(unsigned short int seed16v[3]);
int seed48_r(unsigned short int seed16v[3], struct drand48_data *buffer);
int setenv(const char *name, const char *value, int replace);
int unsetenv(const char *name);
int clearenv(void);
int putenv(char *string);
void srand(unsigned int seed);
void srand48(long seedval);
int srand48_r(long int seedval, struct drand48_data *buffer);
double strtod(char *str, char **ptr);
long double strtod_l(const char* str, char** end, locale_t loc);
float strtof(const char *str, char **end);
long double strtof_l(const char* str, char** end, locale_t loc);
long int strtol(const char *str, char **end, int base);
long int strtol_l(const char *str, char **end, int base, locale_t loc);
long long int strtoll(const char* str, char** end, int base);
long long int strtoll_l(const char* str, char** end, int base, locale_t loc);
unsigned long int strtoul(const char* str, char** end, int base);
unsigned long int strtoul_l(const char* str, char** end, int base, locale_t loc);
unsigned long long int strtoull(const char* str, char** end, int base);
unsigned long long int strtoull_l(const char* str, char** end, int base, locale_t loc);
int system(const char *command);
int grantpt(int fd);
int unlockpt(int fd);
void *valloc(size_t size);
double wcstod(const wchar_t *nptr, wchar_t **endptr);
long double wcstod_l(const wchar_t *nptr, wchar_t **endptr, locale_t loc);
float wcstof(const wchar_t *nptr, wchar_t **endptr);
long double wcstof_l(const wchar_t *nptr, wchar_t **endptr, locale_t loc);
long int wcstol(const wchar_t *nptr, wchar_t **endptr, int base);
long long int wcstol_l(const wchar_t *nptr, wchar_t **endptr, int base, locale_t loc);
long long int wcstoll(const wchar_t *nptr, wchar_t **endptr, int base);
long long int wcstoll_l(const wchar_t *nptr, wchar_t **endptr, int base, locale_t loc);
size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);
unsigned long int wcstoul(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long int wcstoul_l(const wchar_t *nptr, wchar_t **endptr, int base, locale_t loc);
unsigned long long int wcstoull(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long long int wcstoull_l(const wchar_t *nptr, wchar_t **endptr, int base, locale_t loc);
int wctomb(char *s, wchar_t wchar);
char *itoa(int value, char *buffer, int base);

#endif
