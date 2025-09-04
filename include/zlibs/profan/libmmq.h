/*****************************************************************************\
|   === libmmq.h : 2025 ===                                                   |
|                                                                             |
|    Basic C library header for static linking                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_LIBMMQ_H
#define _PROFAN_LIBMMQ_H

#include <stdint.h>
#include <stddef.h>

#define mmq_exit(code) syscall_process_kill(syscall_process_pid(), code)

#define mmq_malloc(size) mmq_malloc_func(size, 0)
#define mmq_malloc_ask(size) mmq_malloc_func(size, 1)

#define mmq_calloc(nmemb, lsize) mmq_calloc_func(nmemb, lsize, 0)
#define mmq_calloc_ask(nmemb, lsize) mmq_calloc_func(nmemb, lsize, 1)

#define mmq_realloc(mem, new_size) mmq_realloc_func(mem, new_size, 0)
#define mmq_realloc_ask(mem, new_size) mmq_realloc_func(mem, new_size, 1)

void  *mmq_calloc_func(uint32_t nmemb, uint32_t lsize, int as_kernel);
void   mmq_free(void *mem);
void  *mmq_realloc_func(void *mem, uint32_t new_size, int as_kernel);
void  *mmq_malloc_func(uint32_t size, int as_kernel);

void  *mmq_memset(void *s, int c, size_t n);
void  *mmq_memcpy(void *dest, const void *src, size_t n);
int    mmq_memcmp(const void *s1, const void *s2, size_t n);
void  *mmq_memmove(void *dest, const void *src, size_t n);

int    mmq_strcmp(const char *s1, const char *s2);
char  *mmq_strcpy(char *s1, const char *s2);
size_t mmq_strlen(const char *s);
char  *mmq_strdup(const char *s);
char  *mmq_strncpy(char *s1, const char *s2, size_t n);
char  *mmq_strcat(char *s1, const char *s2);
int    mmq_strncmp(const char *s1, const char *s2, size_t n);
int    mmq_str2int(const char *nptr);

void mmq_putchar(int fd, char c);
void mmq_putstr(int fd, const char *str);
void mmq_putint(int fd, int n);
void mmq_puthex(int fd, uint32_t n);
void mmq_printf(int fd, const char *fmt, ...);

#endif
