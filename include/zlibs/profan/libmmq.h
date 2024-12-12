/*****************************************************************************\
|   === libmmq.h : 2024 ===                                                   |
|                                                                             |
|    Kernel module header for minimalistic libC implementation     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef LIBMMQ_ID
#define LIBMMQ_ID 1001

#include <stdint.h>
#include <stddef.h>

#define kmalloc(size) kmalloc_func(size, 0)
#define kmalloc_ask(size) kmalloc_func(size, 1)

#define kcalloc(nmemb, lsize) kcalloc_func(nmemb, lsize, 0)
#define kcalloc_ask(nmemb, lsize) kcalloc_func(nmemb, lsize, 1)

#define krealloc(mem, new_size) krealloc_func(mem, new_size, 0)
#define krealloc_ask(mem, new_size) krealloc_func(mem, new_size, 1)

#define process_exit(code) syscall_process_kill(syscall_process_pid(), code)

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define kcalloc_func ((void *(*)(uint32_t, uint32_t, int)) get_func_addr(LIBMMQ_ID, 2))
#define kfree ((void (*)(void *)) get_func_addr(LIBMMQ_ID, 3))
#define krealloc_func ((void *(*)(void *, uint32_t, int)) get_func_addr(LIBMMQ_ID, 4))
#define kmalloc_func ((void *(*)(uint32_t, int)) get_func_addr(LIBMMQ_ID, 5))

#define mem_cpy ((void *(*)(void *, const void *, size_t)) get_func_addr(LIBMMQ_ID, 6))
#define mem_cmp ((int (*)(const void *, const void *, size_t)) get_func_addr(LIBMMQ_ID, 7))
#define mem_set ((void *(*)(void *, int, size_t)) get_func_addr(LIBMMQ_ID, 8))
#define mem_move ((void *(*)(void *, const void *, size_t)) get_func_addr(LIBMMQ_ID, 9))
#define str_cmp ((int (*)(const char *, const char *)) get_func_addr(LIBMMQ_ID, 10))
#define str_cpy ((int (*)(char *, const char *)) get_func_addr(LIBMMQ_ID, 11))
#define str_len ((size_t (*)(const char *)) get_func_addr(LIBMMQ_ID, 12))
#define str_dup ((char *(*)(const char *)) get_func_addr(LIBMMQ_ID, 13))
#define str_ncpy ((char *(*)(char *, const char *, size_t)) get_func_addr(LIBMMQ_ID, 14))
#define str_cat ((char *(*)(char *, const char *)) get_func_addr(LIBMMQ_ID, 15))
#define str_ncmp ((int (*)(const char *, const char *, size_t)) get_func_addr(LIBMMQ_ID, 16))
#define str_int ((int (*)(const char *)) get_func_addr(LIBMMQ_ID, 17))

#define fd_putchar ((void (*)(int, char)) get_func_addr(LIBMMQ_ID, 18))
#define fd_putstr ((void (*)(int, const char *)) get_func_addr(LIBMMQ_ID, 19))
#define fd_putint ((void (*)(int, int)) get_func_addr(LIBMMQ_ID, 20))
#define fd_puthex ((void (*)(int, uint32_t)) get_func_addr(LIBMMQ_ID, 21))
#define fd_printf ((void (*)(int, const char *, ...)) get_func_addr(LIBMMQ_ID, 22))

#endif
