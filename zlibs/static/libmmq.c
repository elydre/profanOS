/*****************************************************************************\
|   === libmmq.c : 2025 ===                                                   |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define _SYSCALL_CREATE_FUNCS

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/libmmq.h>

#include <stdarg.h>

int profan_syscall(uint32_t id, ...) {
    va_list args;
    int a;

    va_start(args, id);

    uint32_t a1 = va_arg(args, uint32_t);
    uint32_t a2 = va_arg(args, uint32_t);
    uint32_t a3 = va_arg(args, uint32_t);
    uint32_t a4 = va_arg(args, uint32_t);
    uint32_t a5 = va_arg(args, uint32_t);

    asm volatile(
        "int $0x80"
        : "=a" (a)
        : "a" (id), "b" (a1), "c" (a2), "d" (a3), "S" (a4), "D" (a5)
    );

    return a;
}

#define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == '\f' || (c) == '\v')
#define isdigit(c) ((c) >= '0' && (c) <= '9')

void *mmq_memset(void *s, int c, size_t n) {
    register uint8_t *p = (uint8_t *) s;
    register uint8_t v = (uint8_t) c;

    while (n--) {
        *p++ = v;
    }

    return s;
}

void *mmq_memcpy(void *dest, const void *src, size_t n) {
    register uint8_t *d = (uint8_t *) dest;
    register const uint8_t *s = (const uint8_t *) src;

    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

void *mmq_calloc_func(uint32_t nmemb, uint32_t lsize, int as_kernel) {
    uint32_t size = lsize * nmemb;
    void *addr = syscall_mem_alloc(size, as_kernel ? 2 : 1, 0);
    if (addr == 0)
        return NULL;
    mmq_memset(addr, 0, size);
    return addr;
}

void mmq_free(void *mem) {
    if (mem == NULL)
        return;
    syscall_mem_free(mem);
}

void *mmq_realloc_func(void *mem, uint32_t new_size, int as_kernel) {
    if (mem == NULL)
        return (void *) syscall_mem_alloc(new_size, as_kernel ? 2 : 1, 0);

    uint32_t old_size = (uint32_t) syscall_mem_alloc_fetch(mem, 0);
    void *new_addr = syscall_mem_alloc(new_size, as_kernel ? 2 : 1, 0);
    if (new_addr == 0)
        return NULL;

    mmq_memcpy(new_addr, mem, old_size < new_size ? old_size : new_size);
    mmq_free(mem);

    return new_addr;
}

void *mmq_malloc_func(uint32_t size, int as_kernel) {
    return syscall_mem_alloc(size, as_kernel ? 2 : 1, 0);
}


int mmq_memcmp(const void *s1, const void *s2, size_t n) {
    register const uint8_t *r1 = (const uint8_t *) s1;
    register const uint8_t *r2 = (const uint8_t *) s2;

    int r = 0;

    while (n-- && ((r = ((int)(*r1++)) - *r2++) == 0));
    return r;
}


void *mmq_memmove(void *dest, const void *src, size_t n) {
    register uint8_t *d = (uint8_t *) dest;
    register const uint8_t *s = (const uint8_t *) src;

    if (d < s) {
        while (n--)
            *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--)
            *--d = *--s;
    }

    return dest;
}

int mmq_strcmp(register const char *s1, register const char *s2) {
    while (*s1 == *s2++) {
        if (*s1++ == 0)
            return 0;
    }
    return *(unsigned char *) s1 - *(unsigned char *) --s2;
}

char *mmq_strcpy(char *restrict s1, const char *restrict s2) {
    int i;
    for (i = 0; s2[i] != '\0'; ++i)
        s1[i] = s2[i];
    s1[i] = '\0';
    return s1;
}

size_t mmq_strlen(const char *s) {
    register const char *p;

    for (p=s ; *p ; p++);

    return p - s;
}

char *mmq_strdup(const char *s) {
    size_t len = mmq_strlen(s);
    char *d = mmq_malloc_func(len + 1, 0);
    if (d == NULL)
        return NULL;
    mmq_memcpy(d, s, len);
    d[len] = '\0';
    return d;
}

char *mmq_strncpy(char *restrict s1, register const char *restrict s2,
               size_t n) {
    register char *s = s1;

    while (n) {
        if ((*s = *s2) != 0) s2++; /* Need to fill tail with 0s. */
        ++s;
        --n;
    }

    return s1;
}

char *mmq_strcat(char *restrict s1, register const char *restrict s2) {
    size_t i,j;
    for (i = 0; s1[i] != '\0'; i++);
    for (j = 0; s2[j] != '\0'; j++)
        s1[i+j] = s2[j];
    s1[i+j] = '\0';
    return s1;
}

int mmq_strncmp(register const char *s1, register const char *s2, size_t n) {
    if (n == 0) return 0;
    do {
        if (*s1 != *s2++)
            return *(unsigned char *) s1 - *(unsigned char *) --s2;
        if (*s1++ == 0)
            break;
    } while (--n != 0);
    return 0;
}

int mmq_str2int(const char *nptr) {
    int n = 0, neg = 0;

    while (isspace(*nptr))
        nptr++;

    switch (*nptr) {
        case '-':
            neg=1;
            nptr++;
            break;
        case '+':
            nptr++;
            break;
    }

    while (isdigit(*nptr))
        n = 10*n - (*nptr++ - '0');

    return neg ? n : -n;
}

void mmq_putchar(int fd, char c) {
    fm_write(fd, &c, 1);
}

void mmq_putstr(int fd, const char *str) {
    fm_write(fd, (char *) str, mmq_strlen(str));
}

void mmq_putint(int fd, int n) {
    if (n < 0) {
        mmq_putchar(fd, '-');
        n = -n;
    }
    if (n / 10) {
        mmq_putint(fd, n / 10);
    }
    mmq_putchar(fd, n % 10 + '0');
}

void mmq_puthex(int fd, uint32_t n) {
    if (n / 16)
        mmq_puthex(fd, n / 16);
    mmq_putchar(fd, "0123456789abcdef"[n % 16]);
}

void mmq_printf(int fd, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_end(args);

    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            mmq_putchar(fd, fmt[i]);
            continue;
        }
        i++;
        if (fmt[i] == 's') {
            char *tmp = va_arg(args, char *);
            if (tmp == NULL)
                tmp = "(null)";
            mmq_putstr(fd, tmp);
        } else if (fmt[i] == 'c') {
            mmq_putchar(fd, va_arg(args, int));
        } else if (fmt[i] == 'd') {
            mmq_putint(fd, va_arg(args, int));
        } else if (fmt[i] == 'x' || fmt[i] == 'p') {
            mmq_puthex(fd, va_arg(args, uint32_t));
        } else {
            mmq_putchar(fd, '%');
        }
    }
    va_end(args);
}
