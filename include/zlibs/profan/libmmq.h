/*****************************************************************************\
|   === libmmq.h : 2024 ===                                                   |
|                                                                             |
|    One-Header library for minimalist programming                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_LIBMMQ_H
#define _PROFAN_LIBMMQ_H

#include <profan/syscall.h>
#include <profan/filesys.h>

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

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

#define kmalloc(size) kmalloc_func(size, 0)
#define kmalloc_ask(size) kmalloc_func(size, 1)

#define kcalloc(nmemb, lsize) kcalloc_func(nmemb, lsize, 0)
#define kcalloc_ask(nmemb, lsize) kcalloc_func(nmemb, lsize, 1)

#define krealloc(mem, new_size) krealloc_func(mem, new_size, 0)
#define krealloc_ask(mem, new_size) krealloc_func(mem, new_size, 1)

#define process_exit(code) syscall_process_kill(syscall_process_pid(), code)

#define _isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == '\f' || (c) == '\v')
#define _isdigit(c) ((c) >= '0' && (c) <= '9')

void *mem_set(void *s, int c, size_t n) {
    register uint8_t *p = (uint8_t *) s;
    register uint8_t v = (uint8_t) c;

    while (n--) {
        *p++ = v;
    }

    return s;
}

void *mem_cpy(void *dest, const void *src, size_t n) {
    register uint8_t *d = (uint8_t *) dest;
    register const uint8_t *s = (const uint8_t *) src;

    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

void *kcalloc_func(uint32_t nmemb, uint32_t lsize, int as_kernel) {
    uint32_t size = lsize * nmemb;
    void *addr = syscall_mem_alloc(size, as_kernel ? 2 : 1, 0);
    if (addr == 0)
        return NULL;
    mem_set(addr, 0, size);
    return addr;
}

void kfree(void *mem) {
    if (mem == NULL)
        return;
    syscall_mem_free(mem);
}

void *krealloc_func(void *mem, uint32_t new_size, int as_kernel) {
    if (mem == NULL)
        return (void *) syscall_mem_alloc(new_size, as_kernel ? 2 : 1, 0);

    uint32_t old_size = (uint32_t) syscall_mem_alloc_fetch(mem, 0);
    void *new_addr = syscall_mem_alloc(new_size, as_kernel ? 2 : 1, 0);
    if (new_addr == 0)
        return NULL;

    mem_cpy(new_addr, mem, old_size < new_size ? old_size : new_size);
    kfree(mem);

    return new_addr;
}

void *kmalloc_func(uint32_t size, int as_kernel) {
    return syscall_mem_alloc(size, as_kernel ? 2 : 1, 0);
}


int mem_cmp(const void *s1, const void *s2, size_t n) {
    register const uint8_t *r1 = (const uint8_t *) s1;
    register const uint8_t *r2 = (const uint8_t *) s2;

    int r = 0;

    while (n-- && ((r = ((int)(*r1++)) - *r2++) == 0));
    return r;
}


void *mem_move(void *dest, const void *src, size_t n) {
    register uint8_t *d = (uint8_t *) dest;
    register const uint8_t *s = (const uint8_t *) src;

    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }

    return dest;
}

int str_cmp(register const char *s1, register const char *s2) {
    while (*s1 == *s2++) {
        if (*s1++ == 0) {
            return 0;
        }
    }
    return *(unsigned char *) s1 - *(unsigned char *) --s2;
}

char *str_cpy(char *restrict s1, const char *restrict s2) {
    int i;
    for (i = 0; s2[i] != '\0'; ++i) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';
    return s1;
}

size_t str_len(const char *s) {
    register const char *p;

    for (p=s ; *p ; p++);

    return p - s;
}

char *str_dup(const char *s) {
    size_t len = str_len(s);
    char *d = kmalloc_func(len + 1, 0);
    if (d == NULL)
        return NULL;
    mem_cpy(d, s, len);
    d[len] = '\0';
    return d;
}

char *str_ncpy(char *restrict s1, register const char *restrict s2,
               size_t n) {
    register char *s = s1;

    while (n) {
        if ((*s = *s2) != 0) s2++; /* Need to fill tail with 0s. */
        ++s;
        --n;
    }

    return s1;
}

char *str_cat(char *restrict s1, register const char *restrict s2) {
    size_t i,j;
    for (i = 0; s1[i] != '\0'; i++);
    for (j = 0; s2[j] != '\0'; j++)
        s1[i+j] = s2[j];
    s1[i+j] = '\0';
    return s1;
}

int str_ncmp(register const char *s1, register const char *s2, size_t n) {
    if (n == 0) return 0;
    do {
        if (*s1 != *s2++)
            return *(unsigned char *) s1 - *(unsigned char *) --s2;
        if (*s1++ == 0)
            break;
    } while (--n != 0);
    return 0;
}

int str_int(const char *nptr) {
    int n = 0, neg = 0;

    while (_isspace(*nptr))
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

    while (_isdigit(*nptr))
        n = 10*n - (*nptr++ - '0');

    return neg ? n : -n;
}

void fd_putchar(int fd, char c) {
    fm_write(fd, &c, 1);
}

void fd_putstr(int fd, const char *str) {
    fm_write(fd, (char *) str, str_len(str));
}

void fd_putint(int fd, int n) {
    if (n < 0) {
        fd_putchar(fd, '-');
        n = -n;
    }
    if (n / 10) {
        fd_putint(fd, n / 10);
    }
    fd_putchar(fd, n % 10 + '0');
}

void fd_puthex(int fd, uint32_t n) {
    if (n / 16) {
        fd_puthex(fd, n / 16);
    }
    fd_putchar(fd, "0123456789abcdef"[n % 16]);
}

void fd_printf(int fd, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_end(args);

    for (int i = 0; fmt[i] != '\0';) {
        if (fmt[i] == '%') {
            i++;
            if (fmt[i] == 's') {
                char *tmp = va_arg(args, char *);
                if (tmp == NULL)
                    tmp = "(null)";
                fd_putstr(fd, tmp);
            } else if (fmt[i] == 'c') {
                fd_putchar(fd, va_arg(args, int));
            } else if (fmt[i] == 'd') {
                fd_putint(fd, va_arg(args, int));
            } else if (fmt[i] == 'x' || fmt[i] == 'p') {
                fd_puthex(fd, va_arg(args, uint32_t));
            } else {
                fd_putchar(fd, '%');
            }
        } else {
            fd_putchar(fd, fmt[i]);
        }
        i++;
    }
    va_end(args);
}

#endif
