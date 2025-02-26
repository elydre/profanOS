/*****************************************************************************\
|   === minilib.h : 2024 ===                                                  |
|                                                                             |
|    Kernel mini-library                                           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef MINILIB_H
#define MINILIB_H

#define UNUSED(x) (void)(x)
#define ARYLEN(x) (int)(sizeof(x) / sizeof((x)[0]))

#define kprintf(...) kprintf_buf((char *) 0, __VA_ARGS__)
#define kprintf_serial(...) kprintf_buf((char *) 1, __VA_ARGS__)

// string functions
void str_cat(char *s1, const char *s2);
int  str_len(const char *s);
void str_cpy(char *s1, const char *s2);
void str_ncpy(char *s1, const char *s2, int n);
void int2str(int n, char *s);
int  str2int(const char *s);
int  str_cmp(const char *s1, const char *s2);
int  str_ncmp(const char *s1, const char *s2, int n);

// memory functions
#define mem_copy(dest, source, nbytes) mem_move(dest, source, nbytes)
void mem_set(void *dest, uint8_t val, uint32_t len);
void mem_move(void *dest, const void *source, uint32_t nbytes);
int mem_cmp(const void *s1, const void *s2, uint32_t n);

void  free(void *addr);
void *malloc(uint32_t size);
void *realloc_as_kernel(void *ptr, uint32_t size);
void *calloc(uint32_t size);

// io format functions
void kprintf_va2buf(char *char_buffer, const char *fmt, va_list args);
void kprintf_buf(char *char_buffer, const char *fmt, ...);

void status_print(int (*func)(), const char *msg);
void kinput(char *buffer, int size);

#endif
