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

#define BUDDY_ALLOC_IMPLEMENTATION
#include "buddy_alloc.h"
#undef BUDDY_ALLOC_IMPLEMENTATION

struct buddy *g_buddy;

#define PROFAN_BUDDY ((void *) 0xD0000000)

void __buddy_init(void) {
    syscall_serial_write(SERIAL_PORT_A, "buddy_init\n", 11);

    size_t arena_size = 1024 * 1024;

    void *ret = syscall_scuba_generate(PROFAN_BUDDY, arena_size / 4096);
    if (ret == NULL) {
        syscall_serial_write(SERIAL_PORT_A, "scuba_generate failed\n", 22);
        return;
    }

    g_buddy = buddy_embed(PROFAN_BUDDY, arena_size);

    if (g_buddy == NULL)
        syscall_serial_write(SERIAL_PORT_A, "buddy_embed failed\n", 19);
}

void __buddy_fini(void) {
    // free(buddy_arena);
}

void malloc_debug_print(void) {
    buddy_debug(g_buddy);
}

void *malloc(uint32_t size) {
    return buddy_malloc(g_buddy, size);
}

void *calloc(uint32_t nmemb, uint32_t lsize) {
    return buddy_calloc(g_buddy, nmemb, lsize);
}

void *realloc(void *mem, uint32_t new_size) {
    return buddy_realloc(g_buddy, mem, new_size, 0);
}

void free(void *mem) {
    if (mem == NULL)
        return;
    switch (buddy_safe_free(g_buddy, mem, SIZE_MAX)) {
        case BUDDY_SAFE_FREE_SUCCESS:
            break;
        case BUDDY_SAFE_FREE_INVALID_ADDRESS:
            fprintf(stderr, "free(%p): invalid pointer\n", mem);
            break;
        case BUDDY_SAFE_FREE_ALREADY_FREE:
            fprintf(stderr, "free(%p): double free\n", mem);
            break;
        default:
            fprintf(stderr, "free(%p): internal error\n", mem);
            break;
    }
}
