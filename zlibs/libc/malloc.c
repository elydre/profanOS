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

#define DEFAULT_SIZE 16  // 8 pages (32KB)

struct buddy *g_buddy;
uint32_t g_arena_count;  // in pages (4KB)
uint32_t g_mdata_count;  // in pages (4KB)

#define PROFAN_BUDDY_MDATA ((void *) 0xD0000000)
#define PROFAN_BUDDY_ARENA ((void *) 0xD0100000)

#include <profan.h>

void __buddy_init(void) {
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

static void extend_virtual(uint32_t size) {
    uint32_t req = g_arena_count + size / 2048;

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

void *malloc(uint32_t size) {
    void *p = buddy_malloc(g_buddy, size);
    if (p) return p;

    extend_virtual(size);
    p = buddy_malloc(g_buddy, size);
    if (p) return p;
    serial_debug("malloc failed\n");
    return NULL;
}

void *calloc(uint32_t nmemb, uint32_t lsize) {
    void *p = buddy_calloc(g_buddy, nmemb, lsize);
    if (p) return p;

    extend_virtual(nmemb * lsize);
    p = buddy_calloc(g_buddy, nmemb, lsize);
    if (p) return p;
    serial_debug("calloc failed\n");
    return NULL;
}

void *realloc(void *mem, uint32_t new_size) {
    void *p = buddy_realloc(g_buddy, mem, new_size, 0);
    if (p) return p;

    extend_virtual(new_size);
    p = buddy_realloc(g_buddy, mem, new_size, 0);
    if (p) return p;
    serial_debug("realloc failed\n");
    return NULL;
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
