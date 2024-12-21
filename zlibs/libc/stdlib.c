/*****************************************************************************\
|   === stdlib.c : 2024 ===                                                   |
|                                                                             |
|    Implementation of stdlib functions from libC                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#include "config_libc.h"

static uint32_t g_rand_seed = 0;

static void **g_atexit_funcs = NULL;
static void *g_entry_exit = NULL;

char **environ = NULL;

/*******************************
 *                            *
 *   CALL BY DYNAMIC LINKER   *
 *                            *
*******************************/

void __buddy_disable_leaks(void);
void __buddy_init(void);

void __stdio_init(void);
void __stdio_fini(void);

void __attribute__((constructor)) __libc_constructor(void) {
    __buddy_init();
    __stdio_init();
}

void __attribute__((destructor)) __libc_destructor(void) {
    __buddy_disable_leaks();
    __stdio_fini();
}

/*******************************
 *                            *
 *   CALL BY ENTRY FUNCTION   *
 *                            *
*******************************/

void __init_libc(char **env, void *entry_exit) {
    int size, offset;

    g_entry_exit = entry_exit;

    if (env == NULL)
        return;

    // check if the libc environment is already initialized
    if (environ != NULL) {
        for (offset = 0; environ[offset] != NULL; offset++);
    } else {
        offset = 0;
    }

    // copy the new environment
    for (size = 0; env[size] != NULL; size++);

    environ = realloc(environ, (size + offset + 1) * sizeof(char *));
    for (int i = 0; i < size; i++)
        environ[i + offset] = strdup(env[i]);
    environ[size + offset] = NULL;

    // set working directory
    char *wd = getenv("PWD");

    if (wd != NULL)
        chdir(wd);
}

void __exit_libc(void) {
    if (g_atexit_funcs) {
        for (int i = 0; g_atexit_funcs[i] != NULL; i++) {
            void (*func)() = g_atexit_funcs[i];
            func();
        }
        free(g_atexit_funcs);
    }

    // free the environment
    if (environ) {
        for (int i = 0; environ[i] != NULL; i++)
            free(environ[i]);
        free(environ);
    }
}

// retro compatibility (lul)
char **__get_environ_ptr(void) {
    return environ;
}

#define TABLE_BASE 0x2e
#define TABLE_SIZE 0x4d
#define XX ((char)0x40)

static const char a64l_table[TABLE_SIZE] = {
  /* 0x2e */                                                           0,  1,
  /* 0x30 */   2,  3,  4,  5,  6,  7,  8,  9, 10, 11, XX, XX, XX, XX, XX, XX,
  /* 0x40 */  XX, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
  /* 0x50 */  27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, XX, XX, XX, XX, XX,
  /* 0x60 */  XX, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
  /* 0x70 */  53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
};

long int a64l(const char *string) {
    const char *ptr = string;
    unsigned long int result = 0ul;
    const char *end = ptr + 6;
    int shift = 0;

    do {
        unsigned index;
        unsigned value;

        index = *ptr - TABLE_BASE;
        if ((unsigned int) index >= TABLE_SIZE)
        break;
        value = (int) a64l_table[index];
        if (value == (int) XX)
            break;
        ++ptr;
        result |= value << shift;
        shift += 6;
    } while (ptr != end);

    return (long int) result;
}

void abort(void) {
    write(2, "== abort ==\n", 12);
    _exit(1);
}

int abs(int j) {
    return (j >= 0) ? j : -j;
}

void atexit(void (*func)()) {
    if (g_atexit_funcs == NULL) {
        g_atexit_funcs = calloc(2, sizeof(void *));
        g_atexit_funcs[0] = func;
    } else {
        int i = 0;
        while (g_atexit_funcs[i] != NULL) i++;
        g_atexit_funcs = realloc(g_atexit_funcs, (i + 2) * sizeof(void *));
        g_atexit_funcs[i] = func;
        g_atexit_funcs[i + 1] = NULL;
    }
}

double atof(const char *s) {
    // This function stolen from either Rolf Neugebauer or Andrew Tolmach.

    double a = 0.0;
    int e = 0;
    int c;
    while ((c = *s++) != '\0' && isdigit(c)) {
        a = a*10.0 + (c - '0');
    }
    if (c == '.') {
        while ((c = *s++) != '\0' && isdigit(c)) {
        a = a*10.0 + (c - '0');
        e = e-1;
        }
    }
    if (c == 'e' || c == 'E') {
        int sign = 1;
        int i = 0;
        c = *s++;
        if (c == '+')
            c = *s++;
        else if (c == '-') {
            c = *s++;
            sign = -1;
        }
        while (isdigit(c)) {
            i = i*10 + (c - '0');
            c = *s++;
        }
        e += i*sign;
    }
    while (e > 0) {
        a *= 10.0;
        e--;
    }
    while (e < 0) {
        a *= 0.1;
        e++;
    }
    return a;
}

int atoi(const char *nptr) {
    int n=0, neg=0;
    while (isspace(*nptr)) nptr++;
    switch (*nptr) {
        case '-': {neg=1; nptr++; break;}
        case '+': {nptr++; break;}
    }
    /* Compute n as a negative number to avoid overflow on INT_MIN */
    while (isdigit(*nptr))
        n = 10*n - (*nptr++ - '0');
    return neg ? n : -n;
}

long atol(const char *nptr) {
    profan_nimpl("atol");
    return 0;
}

long long atoll(const char *nptr) {
    profan_nimpl("atoll");
    return 0;
}

void *bsearch(register const void *key, const void *base0, size_t nmemb, register size_t size,
                register int (*compar)(const void *, const void *)) {
    register const char *base = base0;
    register const void *p;
    register size_t lim;
    register int cmp;

    for (lim = nmemb; lim != 0; lim >>= 1) {
        p = base + (lim >> 1) * size;
        cmp = (*compar)(key, p);
        if (cmp == 0)
            return ((void *) p);
        if (cmp > 0) {  // key > p: move right
            base = (char *) p + size;
            lim--;
        }   // else move left
    }
    return NULL;
}

int clearenv(void) {
    if (environ == NULL)
        return 0;

    for (int i = 0; environ[i] != NULL; i++)
        free(environ[i]);
    free(environ);

    environ = NULL;
    return 0;
}

div_t div(int numer, int denom) {
    div_t result;
    result.quot = numer / denom;
    result.rem  = numer - (result.quot * denom);
    return(result);
}

void exit(int rv) {
    if (g_entry_exit != NULL) {
        void (*entry_exit)(int) = g_entry_exit;
        entry_exit(rv);
    }
    fputs("no entry_exit function found\n", stderr);
    _exit(rv); // unistd
}

char *getenv(const char *var) {
    if (environ == NULL)
        return NULL;
    // check if the variable already exists
    for (int i = 0; environ[i] != NULL; i++) {
        for (int j = 0; ; j++) {
            if (var[j] == '\0' && environ[i][j] == '=') {
                // found the variable
                return environ[i] + j + 1;
            }
            if (var[j] != environ[i][j]) break;
        }
    }
    return NULL;
}

int grantpt(int fd) {
    profan_nimpl("grantpt");
    return 0;
}

static const char l64a_conv_table[64] = {
  '.', '/', '0', '1', '2', '3', '4', '5',
  '6', '7', '8', '9', 'A', 'B', 'C', 'D',
  'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
  'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b',
  'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
  'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
  's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

char *l64a(long int n) {
    unsigned long int m = (unsigned long int) n;
    static char result[7];
    char *p;

    // The standard says that only 32 bits are used
    if (sizeof(m) != 4)
        m &= 0xffffffff;

    // The value for n == 0 is defined to be the empty string
    p = result;
    while (m) {
        *p++ = l64a_conv_table[m & 0x3f];
        m >>= 6;
    }
    *p = '\0';
    return result;
}

long int labs(long int j) {
    return (j >= 0) ? j : -j;
}

ldiv_t ldiv(long int numer, long int denom) {
    profan_nimpl("ldiv");
    ldiv_t result;
    result = (ldiv_t) {0, 0}; // temporary, to avoid warnings
    return result;
}

long long int llabs(long long int j) {
    return (j >= 0) ? j : -j;
}

lldiv_t lldiv(long long int numer, long long int denom) {
    profan_nimpl("lldiv");
    lldiv_t result;
    result = (lldiv_t) {0, 0}; // temporary, to avoid warnings
    return result;
}

int mblen(register const char *s, size_t n) {
    profan_nimpl("mblen");
    return 0;
}

size_t mbstowcs(wchar_t * restrict pwcs, const char * restrict s, size_t n) {
    profan_nimpl("mbstowcs");
    return 0;
}

int mbtowc(wchar_t *restrict wc, const char *restrict src, size_t n) {
    profan_nimpl("mbtowc");
    return 0;
}

char *mkstemp(char *template) {
    profan_nimpl("mkstemp");
    return NULL;
}

char *mktemp(char *template) {
    profan_nimpl("mktemp");
    return NULL;
}

int putenv(char *string) {
    profan_nimpl("putenv");
    return 0;
}

void qsort(void *base, size_t nel, size_t width, int (*comp)(const void *, const void *)) {
    // bubble sort, can be improved
    char *arr = (char *) base;
    char temp[width];
    for (size_t i = 0; i < nel; i++) {
        for (size_t j = 0; j < nel - i - 1; j++) {
            if (comp(arr + j * width, arr + (j + 1) * width) < 0)
                continue;
            memcpy(temp, arr + j * width, width);
            memcpy(arr + j * width, arr + (j + 1) * width, width);
            memcpy(arr + (j + 1) * width, temp, width);
        }
    }
}

int rand(void) {
    return rand_r(&g_rand_seed) & RAND_MAX;
}

int rand_r(unsigned int *seed) {
    unsigned int next = *seed;
    int result;

    next *= 1103515245;
    next += 12345;
    result = (unsigned int) (next / 65536) % 2048;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    *seed = next;

    return result;
}

char *realpath(const char *path, char *resolved_path) {
    profan_nimpl("realpath");
    return NULL;
}

int setenv(const char *name, const char *value, int replace) {
    if (environ == NULL) {
        environ = malloc(sizeof(char *));
        environ[0] = NULL;
    }

    int i;
    // check if the variable already exists
    for (i = 0; environ[i] != NULL; i++) {
        for (int j = 0;; j++) {
            if (name[j] == '\0' && environ[i][j] == '=') {
                // found the variable
                if (!replace)
                    return 0;
                // replace the variable
                free(environ[i]);
                environ[i] = malloc(strlen(name) + strlen(value) + 2);
                strcpy(environ[i], name);
                strcat(environ[i], "=");
                strcat(environ[i], value);
                return 0;
            }
            if (name[j] != environ[i][j] || name[j] == '\0')
                break;
        }
    }

    // the variable doesn't exist, create it
    char *new_var = malloc(strlen(name) + strlen(value) + 2);
    strcpy(new_var, name);
    strcat(new_var, "=");
    strcat(new_var, value);

    // add the variable to the environment
    environ = realloc(environ, (i + 2) * sizeof(char *));
    environ[i] = new_var;
    environ[i + 1] = NULL;

    return 0;
}

void srand(unsigned int seed) {
    g_rand_seed = seed;
}

int system(const char *command) {
    if (access(SYSTEM_SHELL_PATH, X_OK) == -1) {
        fputs("libc: system: '" SYSTEM_SHELL_PATH "' not found\n", stderr);
        return -1;
    }

    return run_ifexist(SYSTEM_SHELL_PATH, 3, ((char *[]) {SYSTEM_SHELL_PATH, "-c", (char *) command}));
}

int unlockpt(int fd) {
    profan_nimpl("unlockpt");
    return 0;
}

int unsetenv(const char *name) {
    if (environ == NULL)
        return 0;
    // check if the variable already exists
    for (int i = 0; environ[i] != NULL; i++) {
        for (int j = 0; ; j++) {
            if (name[j] == '\0' && environ[i][j] == '=') {
                // found the variable
                free(environ[i]);
                for (int k = i; environ[k] != NULL; k++)
                    environ[k] = environ[k + 1];
                return 0;
            }
            if (name[j] != environ[i][j] || name[j] == '\0')
                break;
        }
    }
    return 0;
}

void *valloc(size_t size) {
    profan_nimpl("valloc");
    return NULL;
}

size_t wcstombs(char * restrict s, const wchar_t * restrict pwcs, size_t n) {
    profan_nimpl("wcstombs");
    return 0;
}

int wctomb(char *s, wchar_t wchar) {
    profan_nimpl("wctomb");
    return 0;
}

double strtod(const char *str, char **ptr) {
    char *p;

    if (ptr == (char **) 0)
        return atof(str);

    p = (char *) str;

    while (isspace(*p))
        ++p;

    if (*p == '+' || *p == '-')
        ++p;

    /* INF or INFINITY.  */
    if ((p[0] == 'i' || p[0] == 'I') &&
        (p[1] == 'n' || p[1] == 'N') &&
        (p[2] == 'f' || p[2] == 'F')
    ) {
        if ((p[3] == 'i' || p[3] == 'I') &&
            (p[4] == 'n' || p[4] == 'N') &&
            (p[5] == 'i' || p[5] == 'I') &&
            (p[6] == 't' || p[6] == 'T') &&
            (p[7] == 'y' || p[7] == 'Y')
        ) {
            *ptr = p + 8;
            return atof(str);
        } else {
            *ptr = p + 3;
            return atof(str);
        }
    }

    /* NAN or NAN(foo).  */
    if ((p[0] == 'n' || p[0] == 'N') &&
        (p[1] == 'a' || p[1] == 'A') &&
        (p[2] == 'n' || p[2] == 'N')
    ) {
        p += 3;
        if (*p == '(') {
            ++p;
            while (*p != '\0' && *p != ')')
                ++p;
            if (*p == ')')
                ++p;
        }
        *ptr = p;
        return atof(str);
    }

    /* digits, with 0 or 1 periods in it.  */
    if (isdigit(*p) || *p == '.') {
        int got_dot = 0;
        while (isdigit(*p) || (!got_dot && *p == '.')) {
        if (*p == '.')
            got_dot = 1;
        ++p;
        }

        /* Exponent.  */
        if (*p == 'e' || *p == 'E') {
            int i;
            i = 1;
            if (p[i] == '+' || p[i] == '-')
                ++i;
            if (isdigit(p[i])) {
                while (isdigit(p[i]))
                    ++i;
                *ptr = p + i;
                return atof(str);
            }
        }
        *ptr = p;
        return atof(str);
    }
    /* Didn't find any digits.  Doesn't look like a number.  */
    *ptr = (char *) str;
    return 0.0;
}

long double strtold(const char *str, char **end) {
    profan_nimpl("strtold");
    return 0;
}

float strtof(const char *str, char **end) {
    return (float) strtod((char *) str, end);
}

long int strtol(const char *str, char **end, int base) {
    const char *s;
    unsigned long acc;
    char c;
    unsigned long cutoff;
    int neg, any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    s = str;
    do {
        c = *s++;
    } while (isspace((unsigned char) c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X') &&
        ((s[1] >= '0' && s[1] <= '9') ||
        (s[1] >= 'A' && s[1] <= 'F') ||
        (s[1] >= 'a' && s[1] <= 'f'))) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    acc = any = 0;
    if (base < 2 || base > 36)
        goto noconv;

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set 'any' if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? (unsigned long) - (LONG_MIN + LONG_MAX) + LONG_MAX
        : LONG_MAX;
    cutlim = cutoff % base;
    cutoff /= base;
    for ( ; ; c = *s++) {
        if (c >= '0' && c <= '9')
            c -= '0';
        else if (c >= 'A' && c <= 'Z')
            c -= 'A' - 10;
        else if (c >= 'a' && c <= 'z')
            c -= 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if ((any < 0 || acc > cutoff || acc == cutoff) && c > cutlim)
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LONG_MIN : LONG_MAX;
        errno = ERANGE;
    } else if (!any) {
noconv:
        errno = EINVAL;
    } else if (neg)
        acc = -acc;
    if (end != NULL)
        *end = (char *)(any ? s - 1 : str);
    return acc;
}

long long strtoll(const char *str, char **end, int base) {
    profan_nimpl("strtoll");
    return 0;
}

unsigned long strtoul(const char *nptr, char **endptr, register int base) {
    register const char *s = nptr;
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;

    // See strtol for comments as to the logic used.
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    cutoff = (unsigned long) ULONG_MAX / (unsigned long) base;
    cutlim = (unsigned long) ULONG_MAX % (unsigned long) base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if ((any < 0 || acc > cutoff || acc == cutoff) && c > cutlim)
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = ULONG_MAX;
        errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char *)(any ? s - 1 : nptr);
    return (acc);
}

unsigned long long strtoull(const char *restrict nptr, char **restrict endptr, int base) {
    const char *s;
    unsigned long long acc;
    char c;
    unsigned long long cutoff;
    int neg, any, cutlim;

    s = nptr;
    do {
        c = *s++;
    } while (isspace((unsigned char)c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    acc = any = 0;
    if (base < 2 || base > 36)
        goto noconv;

    cutoff = ULLONG_MAX / base;
    cutlim = ULLONG_MAX % base;

    for ( ; ; c = *s++) {
        if (c >= '0' && c <= '9')
            c -= '0';
        else if (c >= 'A' && c <= 'Z')
            c -= 'A' - 10;
        else if (c >= 'a' && c <= 'z')
            c -= 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = ULLONG_MAX;
        errno = ERANGE;
    } else if (!any) {
noconv:
        errno = EINVAL;
    } else if (neg)
        acc = -acc;
    if (endptr != NULL)
        *endptr = (char *)(any ? s - 1 : nptr);
    return (acc);
}

double wcstod(const wchar_t *nptr, wchar_t **endptr) {
    profan_nimpl("wcstod");
    return 0;
}

float wcstof(const wchar_t *nptr, wchar_t **endptr) {
    profan_nimpl("wcstof");
    return 0;
}

long int wcstol(const wchar_t *nptr, wchar_t **endptr, int base) {
    profan_nimpl("wcstol");
    return 0;
}

long long int wcstoll(const wchar_t *nptr, wchar_t **endptr, int base) {
    profan_nimpl("wcstoll");
    return 0;
}

unsigned long int wcstoul(const wchar_t *nptr, wchar_t **endptr, int base) {
    profan_nimpl("wcstoul");
    return 0;
}

unsigned long long int wcstoull(const wchar_t *nptr, wchar_t **endptr, int base) {
    profan_nimpl("wcstoull");
    return 0;
}

static void I_swap(char *x, char *y) {
    char t = *x;
    *x = *y;
    *y = t;
}

// Function to reverse `buffer[i..j]`
static char *I_reverse(char *buffer, int i, int j) {
    while (i < j) {
        I_swap(&buffer[i++], &buffer[j--]);
    }

    return buffer;
}

char *itoa(int value, char *buffer, int base) {
    // invalid input
    if (base < 2 || base > 32) {
        return buffer;
    }

    // consider the absolute value of the number
    int n = abs(value);

    int i = 0;
    while (n) {
        int r = n % base;

        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        }
        else {
            buffer[i++] = 48 + r;
        }

        n = n / base;
    }

    // if the number is 0
    if (i == 0) {
        buffer[i++] = '0';
    }

    // If the base is 10 and the value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0'; // null terminate string

    // reverse the string and return it
    return I_reverse(buffer, 0, i - 1);
}
