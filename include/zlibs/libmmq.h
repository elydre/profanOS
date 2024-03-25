#ifndef LIBMMQ_ID
#define LIBMMQ_ID 1008

#include <type.h>

#define malloc(size) ((void *) c_mem_alloc((size), 0, 1))
#define exit(code) c_exit_pid(c_process_get_pid(), code, 0)

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define calloc ((void *(*)(uint32_t, uint32_t)) get_func_addr(LIBMMQ_ID, 2))
#define free ((void (*)(void *)) get_func_addr(LIBMMQ_ID, 3))
#define realloc ((void *(*)(void *, uint32_t)) get_func_addr(LIBMMQ_ID, 4))
#define memcpy ((void *(*)(void *, const void *, size_t)) get_func_addr(LIBMMQ_ID, 5))
#define memcmp ((int (*)(const void *, const void *, size_t)) get_func_addr(LIBMMQ_ID, 6))
#define memset ((void *(*)(void *, int, size_t)) get_func_addr(LIBMMQ_ID, 7))
#define memmove ((void *(*)(void *, const void *, size_t)) get_func_addr(LIBMMQ_ID, 8))
#define strcmp ((int (*)(const char *, const char *)) get_func_addr(LIBMMQ_ID, 9))
#define strcpy ((int (*)(char *, const char *)) get_func_addr(LIBMMQ_ID, 10))
#define strlen ((size_t (*)(const char *)) get_func_addr(LIBMMQ_ID, 11))
#define strdup ((char *(*)(const char *)) get_func_addr(LIBMMQ_ID, 12))

#endif
