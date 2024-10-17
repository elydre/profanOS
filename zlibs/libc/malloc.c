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
#include <profan.h>
#include <stdlib.h>

#define BUDDY_ALLOC_IMPLEMENTATION
#include "buddy_alloc.h"
#undef BUDDY_ALLOC_IMPLEMENTATION

#define DEFAULT_SIZE 16  // 8 pages (32KB)

#define PROFAN_BUDDY_MDATA ((void *) 0xD0000000)
#define PROFAN_BUDDY_ARENA ((void *) 0xD0100000)

typedef struct {
    void *ptr;
    uint32_t size;
    void *caller[4];
} leaks_debug_t;

leaks_debug_t *g_leaks_debug;
struct buddy  *g_buddy;

uint32_t g_arena_count;  // in pages (4KB)
uint32_t g_mdata_count;  // in pages (4KB)
int      g_leaks;

void __buddy_init(void) {
    g_leaks = 0;

    g_arena_count = DEFAULT_SIZE;
    if (!syscall_scuba_generate(PROFAN_BUDDY_ARENA, g_arena_count)) {
        syscall_serial_write(SERIAL_PORT_A, "scuba_generate failed\n", 22);
        return;
    }

    g_mdata_count = buddy_sizeof(g_arena_count * 4096) / 4096 + 1;
    if (!syscall_scuba_generate(PROFAN_BUDDY_MDATA, g_mdata_count)) {
        syscall_serial_write(SERIAL_PORT_A, "scuba_generate failed\n", 22);
        return;
    }

    g_buddy = buddy_init(PROFAN_BUDDY_MDATA, PROFAN_BUDDY_ARENA, g_arena_count * 4096);

    if (g_buddy == NULL)
        syscall_serial_write(SERIAL_PORT_A, "buddy_embed failed\n", 19);
}

void __buddy_fini(void) {
    return;
}

void __buddy_enable_leaks(void) {
    g_leaks_debug = calloc(1, sizeof(leaks_debug_t));
    g_leaks = 1;
}

void __buddy_show_leaks(void) {
    for (int i = 0; i < g_leaks; i++) {
        if (g_leaks_debug[i].ptr) {
            fprintf(stderr, "leak: %p %d\n", g_leaks_debug[i].ptr, g_leaks_debug[i].size);
        }
    }
}

static void register_alloc(void *ptr, uint32_t size) {
    serial_debug("register_alloc: %p %d\n", ptr, size);

    for (int i = 0; i < g_leaks; i++) {
        if (g_leaks_debug[i].ptr)
            continue;
        g_leaks_debug[i].size = size;
        g_leaks_debug[i].ptr = ptr;
        return;
    }

    int leaks_size = g_leaks * 2;
    g_leaks = 0;
    g_leaks_debug = realloc(g_leaks_debug, leaks_size * sizeof(leaks_debug_t));
    g_leaks = leaks_size;

    g_leaks_debug[g_leaks].size = size;
    g_leaks_debug[g_leaks].ptr = ptr;
}

static void register_free(void *ptr) {
    serial_debug("register_free: %p\n", ptr);

    for (int i = 0; i < g_leaks; i++) {
        if (g_leaks_debug[i].ptr == ptr) {
            g_leaks_debug[i].ptr = NULL;
            return;
        }
    }
}

static void extend_virtual(uint32_t size) {
    uint32_t req = g_arena_count + size / 2048 + 1;

    // align to the next power of 2 (11 -> 16)
    while (req & (req - 1)) {
        req += req & -req;
    }

    serial_debug("extend_virtual: %d -> %d\n", g_arena_count, req);

    if (!syscall_scuba_generate(PROFAN_BUDDY_ARENA + g_arena_count * 4096, req - g_arena_count)) {
        syscall_serial_write(SERIAL_PORT_A, "scuba_generate failed\n", 22);
        return;
    }

    // grow the metadata if needed
    uint32_t mdata_count = buddy_sizeof(req * 4096) / 4096 + 1;
    if (mdata_count > g_mdata_count) {
        mdata_count *= 2; // avoid too many resizes
        if (mdata_count > (PROFAN_BUDDY_ARENA - PROFAN_BUDDY_MDATA) / 4096) {
            serial_debug("too many metadata pages\n");
            return;
        }
        serial_debug("extend_virtual: mdata %d -> %d\n", g_mdata_count, mdata_count);
        if (!syscall_scuba_generate(PROFAN_BUDDY_MDATA + g_mdata_count * 4096, mdata_count - g_mdata_count)) {
            syscall_serial_write(SERIAL_PORT_A, "scuba_generate failed\n", 22);
            return;
        }
        g_mdata_count = mdata_count;
    }

    g_arena_count = req;

    g_buddy = buddy_resize(g_buddy, g_arena_count * 4096);

    if (g_buddy == NULL)
        syscall_serial_write(SERIAL_PORT_A, "buddy_resize failed\n", 20);
}

#define ALLOC_DO(ptr, size)         \
    if (ptr) {                      \
        if (g_leaks)                \
            register_alloc(ptr, size);   \
        return ptr;                 \
    }

void *malloc(uint32_t size) {
    void *p = buddy_malloc(g_buddy, size);
    ALLOC_DO(p, size);

    extend_virtual(size);

    p = buddy_malloc(g_buddy, size);
    ALLOC_DO(p, size);

    serial_debug("malloc failed\n");
    return NULL;
}

void *calloc(uint32_t nmemb, uint32_t lsize) {
    void *p = buddy_calloc(g_buddy, nmemb, lsize);
    ALLOC_DO(p, nmemb * lsize);

    extend_virtual(nmemb * lsize);

    p = buddy_calloc(g_buddy, nmemb, lsize);
    ALLOC_DO(p, nmemb * lsize);

    serial_debug("calloc failed\n");
    return NULL;
}

void *realloc(void *mem, uint32_t new_size) {
    void *p = buddy_realloc(g_buddy, mem, new_size, 0);
    ALLOC_DO(p, new_size);

    extend_virtual(new_size);

    p = buddy_realloc(g_buddy, mem, new_size, 0);
    ALLOC_DO(p, new_size);

    serial_debug("realloc failed\n");
    return NULL;
}

void free(void *mem) {
    if (mem == NULL)
        return;
    switch (buddy_safe_free(g_buddy, mem, SIZE_MAX)) {
        case BUDDY_SAFE_FREE_SUCCESS:
            if (g_leaks)
                register_free(mem);
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
