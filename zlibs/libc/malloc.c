/*****************************************************************************\
|   === malloc.c : 2024 ===                                                   |
|                                                                             |
|    Implementation of malloc functions from libC                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <string.h>

void free(void *mem) {
    if (mem == NULL)
        return;
    syscall_mem_free_addr((uint32_t) mem);
}

void *malloc(uint32_t size) {
    return (void *) syscall_mem_alloc(size, 0, 1);
}

void *malloc_ask(uint32_t size) {
    return (void *) syscall_mem_alloc(size, 0, 6);
}

void *calloc(uint32_t nmemb, uint32_t lsize) {
    uint32_t size = lsize * nmemb;
    void *addr = (void *) syscall_mem_alloc(size, 0, 1);

    if (addr == NULL)
        return NULL;

    memset(addr, 0, size);
    return addr;
}

void *calloc_ask(uint32_t nmemb, uint32_t lsize) {
    uint32_t size = lsize * nmemb;
    void *addr = (void *) syscall_mem_alloc(size, 0, 6);

    if (addr == NULL)
        return NULL;

    memset(addr, 0, size);
    return addr;
}

void *realloc(void *mem, uint32_t new_size) {
    if (mem == NULL)
        return (void *) syscall_mem_alloc(new_size, 0, 1);

    uint32_t old_size = syscall_mem_get_alloc_size((uint32_t) mem);
    void *new_addr = (void *) syscall_mem_alloc(new_size, 0, 1);

    if (new_addr == NULL)
        return NULL;

    memcpy(new_addr, mem, old_size < new_size ? old_size : new_size);
    free(mem);
    return new_addr;
}

void *realloc_ask(void *mem, uint32_t new_size) {
    if (mem == NULL)
        return (void *) syscall_mem_alloc(new_size, 0, 6);

    uint32_t old_size = syscall_mem_get_alloc_size((uint32_t) mem);
    void *new_addr = (void *) syscall_mem_alloc(new_size, 0, 6);

    if (new_addr == NULL)
        return NULL;

    memcpy(new_addr, mem, old_size < new_size ? old_size : new_size);
    free(mem);
    return new_addr;
}
