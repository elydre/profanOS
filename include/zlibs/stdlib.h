/*****************************************************************************\
|   === stdlib.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the stdlib.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define RAND_MAX 0x7fffffff

extern char **environ;

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

// non standard (GNU/BSD extension)
long int  a64l(const char *string);
char     *l64a(long int n);
int       rand_r(unsigned int *seed);
char     *itoa(int value, char *buffer, int base);
char     *realpath(const char *path, char *resolved_path);

// standard functions
void      abort(void) __attribute__((noreturn));
int       abs(int j);
void      atexit(void (*func)(void));
double    atof(const char *s);
int       atoi(const char *nptr);
long      atol(const char *nptr);
long long atoll(const char *nptr);
void     *bsearch(const void *key, const void *base, size_t high, size_t size, int (*compar)(const void *, const void *));
void     *calloc(size_t nmemb, size_t lsize);
int       clearenv(void);
div_t     div(int numer, int denom);
void      exit(int rv) __attribute__((noreturn));
void      free(void *mem);
char     *getenv(const char *var);
int       grantpt(int fd);
long int  labs(long int j);
ldiv_t    ldiv(long int numer, long int denom);
long long llabs(long long j);
lldiv_t   lldiv(long long int numer, long long int denom);
void     *malloc(size_t size);
int       mblen(const char *s, size_t n);
size_t    mbstowcs(wchar_t *pwcs, const char *s, size_t n);
int       mbtowc(wchar_t *pwc, const char *s, size_t n);
char     *mkstemp(char *template);
char     *mktemp(char *template);
int       putenv(char *string);
void      qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));
int       rand(void);
void     *realloc(void *mem, size_t new_size);
int       setenv(const char *name, const char *value, int replace);
void      srand(unsigned int seed);
int       system(const char *command);
int       unlockpt(int fd);
int       unsetenv(const char *name);
void     *valloc(size_t size);
int       wctomb(char *s, wchar_t wchar);
size_t    wcstombs(char *s, const wchar_t *pwcs, size_t n);

double             strtod(const char *str, char **ptr);
float              strtof(const char *str, char **end);
long double        strtold(const char *str, char **end);
long               strtol(const char *str, char **end, int base);
long long          strtoll(const char *str, char **end, int base);
unsigned long      strtoul(const char *str, char **end, int base);
unsigned long long strtoull(const char *str, char **end, int base);

double             wcstod(const wchar_t *nptr, wchar_t **endptr);
float              wcstof(const wchar_t *nptr, wchar_t **endptr);
long               wcstol(const wchar_t *nptr, wchar_t **endptr, int base);
long long          wcstoll(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long      wcstoul(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long long wcstoull(const wchar_t *nptr, wchar_t **endptr, int base);

#endif
