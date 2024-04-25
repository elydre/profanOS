/****** This file is part of profanOS **************************\
|   == minilib.h ==                                  .pi0iq.    |
|                                                   d"  . `'b   |
|   Kernel mini-library                             q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef MINILIB_H
#define MINILIB_H

#define UNUSED(x) (void)(x)
#define ARYLEN(x) (int)(sizeof(x) / sizeof((x)[0]))

#define kprintf(...) kprintf_buf((char *) 0, __VA_ARGS__)
#define kprintf_serial(...) kprintf_buf((char *) 1, __VA_ARGS__)

// string functions
void str_cat(char *s1, char *s2);
int  str_len(char *s);
void str_cpy(char *s1, char *s2);
void str_ncpy(char *s1, char *s2, int n);
void int2str(int n, char *s);
int  str2int(char *s);
int  str_cmp(char *s1, char *s2);
int  str_ncmp(char *s1, char *s2, int n);
int  str_count(char *s, char c);
void str_append(char *s, char c);

// memory functions
#define mem_copy(dest, source, nbytes) mem_move(dest, source, nbytes)
void mem_set(void *dest, uint8_t val, uint32_t len);
void mem_move(void *dest, void *source, uint32_t nbytes);

void  free(void *addr);
void *malloc(uint32_t size);
void *realloc_as_kernel(void *ptr, uint32_t size);
void *calloc(uint32_t size);

// io format functions
void kprintf_va2buf(char *char_buffer, char *fmt, va_list args);
void kprintf_buf(char *char_buffer, char *fmt, ...);

void status_print(int (*func)(), char *verb, char *noun);
void kinput(char *buffer, int size);
void krainbow(char *message);

#endif
