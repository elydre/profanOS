#include <syscall.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

uint32_t rand_seed = 0;
char **g_env;

void *malloc_func(uint32_t size, int as_kernel);

#define calloc(nmemb, lsize) calloc_func(nmemb, lsize, 0)
#define malloc(size) malloc_func(size, 0)
#define realloc(mem, new_size) realloc_func(mem, new_size, 0)

#define calloc_ask(nmemb, lsize) calloc_func(nmemb, lsize, 1)
#define malloc_ask(size) malloc_func(size, 1)
#define realloc_ask(mem, new_size) realloc_func(mem, new_size, 1)

#ifndef isdigit
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#endif

#ifndef isspace
#define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')
#endif

#define SHELL_PATH "/bin/fatpath/olivine.bin"

int main(void) {
    rand_seed = time(NULL);
    g_env = malloc(sizeof(char *));
    g_env[0] = NULL;
    return 0;
}

char **get_environ_ptr(void) {
    return g_env;
}

void *calloc_func(uint32_t nmemb, uint32_t lsize, int as_kernel) {
    uint32_t size = lsize * nmemb;
    int addr = c_mem_alloc(size, 0, as_kernel ? 6 : 1);
    if (addr == 0) return NULL;
    memset((uint8_t *) addr, 0, size);
    return (void *) addr;
}

void free(void *mem) {
    if (mem == NULL) return;
    int size = c_mem_get_alloc_size((uint32_t) mem);
    if (size == 0) {
        printf("free(%p) : invalid pointer\n", mem);
        return;
    }
    memset((uint8_t *) mem, 0, size);
    c_mem_free_addr((int) mem);
}

void *malloc_func(uint32_t size, int as_kernel) {
    uint32_t addr = c_mem_alloc(size, 0, as_kernel ? 6 : 1);
    if (addr == 0) return NULL; // error
    return (void *) addr;
}

void *realloc_func(void *mem, uint32_t new_size, int as_kernel) {
    if (mem == NULL) return malloc_func(new_size, as_kernel);
    uint32_t addr = (uint32_t) mem;
    uint32_t new_addr = c_mem_alloc(new_size, 0, as_kernel ? 6 : 1);
    if (new_addr == 0) return NULL;
    memcpy((uint8_t *) new_addr, (uint8_t *) addr, new_size);
    free(mem);
    return (void *) new_addr;
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
    puts("abort not implemented yet, WHY DO YOU USE IT ?");
    // TODO : implement abort, it's totally possible
}

int abs(int j) {
    return (j >= 0) ? j : -j;
}

void atexit(void (*func)()) {
    puts("atexit not implemented yet, WHY DO YOU USE IT ?");
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
    puts("atol not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long long atoll(const char *nptr) {
    puts("atoll not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void *bsearch(const void *key, const void *base, size_t /* nmemb */ high,
              size_t size, int (*compar)(const void *, const void *)) {
    puts("bsearch not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

char *canonicalize_file_name (const char *name) {
    puts("canonicalize_file_name not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

div_t div(int numer, int denom) {
    div_t result;
    result.quot = numer / denom;
    result.rem  = numer - (result.quot * denom);
    return(result);
}

double drand48(void) {
    puts("drand48 not implemented yet, WHY DO YOU USE IT ?");
    return 0.0;
}

int drand48_r(struct drand48_data *buffer, double *result) {
    puts("drand48_r not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int __drand48_iterate(unsigned short int xsubi[3], struct drand48_data *buffer) {
    puts("__drand48_iterate not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

double erand48(unsigned short int xsubi[3]) {
    puts("erand48 not implemented yet, WHY DO YOU USE IT ?");
    return 0.0;
}

int erand48_r(unsigned short int xsubi[3], struct drand48_data *buffer, double *result) {
    puts("erand48_r not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void exit(int rv) {
    c_exit_pid(c_process_get_pid(), rv);
}

char *gcvt(double number, int ndigit, char *buf) {
    puts("gcvt not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

char *getenv(const char *var) {
    // check if the variable already exists
    for (int i = 0; g_env[i] != NULL; i++) {
        for (int j = 0; ; j++) {
            if (var[j] == '\0' && g_env[i][j] == '=') {
                // found the variable
                return g_env[i] + j + 1;
            }
            if (var[j] != g_env[i][j]) break;
        }
    }
    return NULL;
}

int getpt(void) {
    puts("getpt not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long int jrand48(unsigned short int xsubi[3]){
    puts("jrand48 not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int jrand48_r(unsigned short int xsubi[3], struct drand48_data *buffer, long int *result){
    puts("jrand48_r not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

// FROM THE STDLIB :
/* Conversion table.  */
static const char conv_table[64] =
{
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

    /* The standard says that only 32 bits are used.  */
    if (sizeof(m) != 4)
        m &= 0xffffffff;

    /* The value for N == 0 is defined to be the empty string,
    * this code provides that as well. */
    p = result;
    while (m) {
        *p++ = conv_table[m & 0x3f];
        m >>= 6;
    }
    *p = '\0';
    return result;
}

long int labs(long int j) {
    return (j >= 0) ? j : -j;
}

void lcong48(unsigned short int param[7]) {
    puts("lcong48 not implemented yet, WHY DO YOU USE IT ?");
}

ldiv_t ldiv (long int numer, long int denom) {
    puts("ldiv not implemented yet, WHY DO YOU USE IT ?");
    ldiv_t result;
    result = (ldiv_t) {0, 0}; // temporary, to avoid warnings
    return result;
}

long long int llabs(long long int j) {
    return (j >= 0) ? j : -j;
}

lldiv_t lldiv (long long int numer, long long int denom) {
    puts("lldiv not implemented yet, WHY DO YOU USE IT ?");
    lldiv_t result;
    result = (lldiv_t) {0, 0}; // temporary, to avoid warnings
    return result;
}

long int lrand48(void) {
    puts("lrand48 not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int lrand48_r(struct drand48_data *buffer, long int *result) {
    puts("lrand48_r not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int mblen(register const char *s, size_t n) {
    puts("mblen not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

size_t mbstowcs(wchar_t * __restrict pwcs, const char * __restrict s, size_t n) {
    puts("mbstowcs not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int mbtowc(wchar_t *__restrict pwc, register const char *__restrict s, size_t n) {
    puts("mbtowc not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *mkdtemp(char *template) {
    puts("mkdtemp not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

int mkostemp(char *template, int flags) {
    puts("mkostemp not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int mkostemp64(char *template, int flags) {
    puts("mkostemp64 not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int mkostemps(char *template, int suffixlen, int flags) {
    puts("mkostemps not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int mkostemps64(char *template, int suffixlen, int flags) {
    puts("mkostemps64 not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int mkstemp (char *template) {
    puts("mkstemp not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int mkstemp64 (char *template) {
    puts("mkstemp64 not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int mkstemps (char *template, int suffixlen) {
    puts("mkstemps not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int mkstemps64 (char *template, int suffixlen) {
    puts("mkstemps64 not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *mktemp(char *template) {
    puts("mktemp not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

long int mrand48(void) {
    puts("mrand48 not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int mrand48_r(struct drand48_data *buffer, long int *result) {
    puts("mrand48_r not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long int nrand48 (unsigned short int xsubi[3]) {
    puts("nrand48 not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int nrand48_r(unsigned short int xsubi[3], struct drand48_data *buffer, long int *result) {
    puts("nrand48_r not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int on_exit(oefuncp func, void *arg) {
    puts("on_exit not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int posix_memalign(void **memptr, size_t alignment, size_t size) {
    puts("posix_memalign not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *ptsname (int fd) {
    puts("ptsname not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

void qsort(void  *base, size_t nel, size_t width, __compar_fn_t comp) {
    // bubble sort
    char *arr = (char*) base;
    char temp[width];
    for (size_t i = 0; i < nel; i++) {
        for (size_t j = 0; j < nel - i - 1; j++) {
            if (comp(arr + j * width, arr + (j + 1) * width) > 0) {
                memcpy(temp, arr + j * width, width);
                memcpy(arr + j * width, arr + (j + 1) * width, width);
                memcpy(arr + (j + 1) * width, temp, width);
            }
        }
    }
}

void qsort_r(void  *base, size_t nel, size_t width, __compar_d_fn_t comp, void *arg) {
    puts("qsort_r not implemented yet, WHY DO YOU USE IT ?");
}

int rand_r (unsigned int *seed);
int rand(void) {
    return rand_r(&rand_seed);
}

/* This algorithm is mentioned in the ISO C standard, here extended
   for 32 bits.  */
// FROM THE STDLIB
int rand_r (unsigned int *seed) {
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

long int random(void) {
    puts("random not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int random_r(struct random_data *buf, int32_t *result) {
    puts("random_r not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *realpath(const char *path, char *got_path) {
    puts("realpath not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

int rpmatch (const char *__response) {
    puts("rpmatch not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *secure_getenv(const char *name) {
    puts("(OK) secure_getenv not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

unsigned short int *seed48(unsigned short int seed16v[3]) {
    puts("seed48 not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

int seed48_r(unsigned short int seed16v[3], struct drand48_data *buffer) {
    puts("seed48_r not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int setenv(const char *name, const char *value, int replace) {
    // check if the variable already exists
    for (int i = 0; g_env[i] != NULL; i++) {
        for (int j = 0; ; j++) {
            if (name[j] == '\0' && g_env[i][j] == '=') {
                // found the variable
                if (replace) {
                    // replace the variable
                    free(g_env[i]);
                    g_env[i] = malloc_ask(strlen(name) + strlen(value) + 2);
                    strcpy(g_env[i], name);
                    strcat(g_env[i], "=");
                    strcat(g_env[i], value);
                }
                return 0;
            }
            if (name[j] != g_env[i][j]) break;
        }
    }

    // the variable doesn't exist, create it
    int len = strlen(name) + strlen(value) + 2;
    char *new_var = malloc_ask(len);
    strcpy(new_var, name);
    strcat(new_var, "=");
    strcat(new_var, value);

    // add the variable to the environment
    int i = 0;
    while (g_env[i] != NULL) i++;
    g_env = realloc_ask(g_env, (i + 2) * sizeof(char *));
    g_env[i] = new_var;
    g_env[i + 1] = NULL;

    return 0;
}

int unsetenv(const char *name) {
    // check if the variable already exists
    for (int i = 0; g_env[i] != NULL; i++) {
        for (int j = 0; ; j++) {
            if (name[j] == '\0' && g_env[i][j] == '=') {
                // found the variable
                free(g_env[i]);
                for (int k = i; g_env[k] != NULL; k++) {
                    g_env[k] = g_env[k + 1];
                }
                return 0;
            }
            if (name[j] != g_env[i][j]) break;
        }
    }
    return 0;
}

int clearenv(void) {
    for (int i = 0; g_env[i] != NULL; i++) {
        free(g_env[i]);
    }
    realloc_ask(g_env, sizeof(char *));
    g_env[0] = NULL;
    return 0;
}

int putenv(char *string) {
    puts("putenv not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void srand48(long seedval) {
    puts("srand48 not implemented yet, WHY DO YOU USE IT ?");
}

int srand48_r(long int seedval, struct drand48_data *buffer) {
    puts("srand48_r not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

double strtod(char *str, char **ptr) {
    char *p;

    if (ptr == (char **) 0)
        return atof (str);

    p = str;

    while (isspace (*p))
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
            return atof (str);
        } else {
            *ptr = p + 3;
            return atof (str);
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
        return atof (str);
    }

    /* digits, with 0 or 1 periods in it.  */
    if (isdigit (*p) || *p == '.') {
        int got_dot = 0;
        while (isdigit (*p) || (!got_dot && *p == '.')) {
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
            if (isdigit (p[i])) {
                while (isdigit (p[i]))
                    ++i;
                *ptr = p + i;
                return atof (str);
            }
        }
        *ptr = p;
        return atof (str);
    }
    /* Didn't find any digits.  Doesn't look like a number.  */
    *ptr = str;
    return 0.0;
}

long double strtod_l(const char* str, char** end, locale_t loc) {
    puts("strtod_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

float strtof(const char *str, char **end) {
    return (float) strtod((char *) str, end);
}

long double strtof_l(const char* str, char** end, locale_t loc) {
    puts("strtof_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long int strtol(const char* str, char** end, int base) {
    puts("strtol not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long long int strtol_l(const char* str, char** end, int base, locale_t loc) {
    puts("strtol_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}


long long strtoll(const char* str, char** end, int base) {
    puts("strtoll not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long long int strtoll_l(const char* str, char** end, int base, locale_t loc) {
    puts("strtoll_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned long int strtoul(const char* str, char** end, int base) {
    puts("strtoul not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned long long int strtoul_l(const char* str, char** end, int base, locale_t loc) {
    puts("strtoul_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned long long int strtoull(const char* str, char** end, int base) {
    puts("strtoull not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned long long int strtoull_l(const char* str, char** end, int base, locale_t loc) {
    puts("strtoull_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int system(const char *command) {
    // generate the arguments
    char **args = malloc(4 * sizeof(char *));
    args[0] = SHELL_PATH;
    args[1] = "-c";
    args[2] = (char *) command;
    args[3] = NULL;

    // run the command
    int ret = c_run_ifexist(args[0], 3, args);

    free(args);

    return ret;
}

int grantpt(int fd) {
    puts("grantpt not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int unlockpt(int fd) {
    puts("unlockpt not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

#define __ptr_t void *
__ptr_t valloc(size_t size) {
    puts("valloc not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

double wcstod(const wchar_t *nptr, wchar_t **endptr) {
    puts("wcstod not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long double wcstod_l(const wchar_t *nptr, wchar_t **endptr, locale_t loc) {
    puts("wcstod_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

float wcstof(const wchar_t *nptr, wchar_t **endptr) {
    puts("wcstof not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long double wcstof_l(const wchar_t *nptr, wchar_t **endptr, locale_t loc) {
    puts("wcstof_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long int wcstol(const wchar_t *nptr, wchar_t **endptr, int base) {
    puts("wcstol not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long long int wcstol_l(const wchar_t *nptr, wchar_t **endptr, int base, locale_t loc) {
    puts("wcstol_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long long int wcstoll(const wchar_t *nptr, wchar_t **endptr, int base) {
    puts("wcstoll not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long long int wcstoll_l(const wchar_t *nptr, wchar_t **endptr, int base, locale_t loc) {
    puts("wcstoll_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

size_t wcstombs(char * __restrict s, const wchar_t * __restrict pwcs, size_t n) {
    puts("wcstombs not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned long int wcstoul(const wchar_t *nptr, wchar_t **endptr, int base) {
    puts("wcstoul not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned long long int wcstoul_l(const wchar_t *nptr, wchar_t **endptr, int base, locale_t loc) {
    puts("wcstoul_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned long long int wcstoull(const wchar_t *nptr, wchar_t **endptr, int base) {
    puts("wcstoull not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned long long int wcstoull_l(const wchar_t *nptr, wchar_t **endptr, int base, locale_t loc) {
    puts("wcstoull_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int wctomb(char *s, wchar_t wchar) {
    puts("wctomb not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void I_swap(char *x, char *y) {
    char t = *x;
    *x = *y;
    *y = t;
}

// Function to reverse `buffer[i..j]`
char* I_reverse(char *buffer, int i, int j)
{
    while (i < j) {
        I_swap(&buffer[i++], &buffer[j--]);
    }

    return buffer;
}

char* itoa(int value, char* buffer, int base) {
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
