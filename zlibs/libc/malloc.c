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

typedef struct alloc_debug {
    void *ptr;
    uint32_t size;
    void *caller[4];
} alloc_debug_t;

typedef struct {
    struct alloc_debug *allocs;
    uint32_t total_allocs;
    uint32_t total_free;
    uint32_t total_size;
    int tab_size;
} leaks_stat_t;

leaks_stat_t *g_stat;
struct buddy  *g_buddy;

uint32_t g_arena_count;  // in pages (4KB)
uint32_t g_mdata_count;  // in pages (4KB)

int      g_debug;

void __buddy_init(void) {
    g_debug = 0;

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
    g_stat = calloc(1, sizeof(leaks_stat_t));
    g_stat->allocs = calloc(1, sizeof(leaks_stat_t));
    g_stat->tab_size = 1;
    g_debug = 1;
}

void __buddy_show_leaks(void) {
    if (!g_debug)
        return;
    g_debug = 0;

    uint32_t total_leaks = 0;
    uint32_t total_size = 0;

    for (int i = 0; i < g_stat->tab_size; i++) {
        if (g_stat->allocs[i].ptr) {
            fprintf(stderr, "%d BYTES NOT FREED AT %p\n", g_stat->allocs[i].size, g_stat->allocs[i].ptr);
            total_size += g_stat->allocs[i].size;
            total_leaks++;

            for (int j = 0; j < 4; j++) {
                void *ptr = g_stat->allocs[i].caller[j];
                if (ptr == NULL)
                    break;
                serial_debug("name for: 0x%08x\n", ptr);
                char *name = profan_fn_name(ptr);
                serial_debug("name resolution finished %p\n", name);
                fprintf(stderr, "  0x%08x %s\n", ptr, name ? name : "???");
            }
            fputs("  ...\n\n", stderr);
        }
    }
    fputs("MEMORY USAGE SUMMARY:\n", stderr);
    fprintf(stderr, "  Unfreed at exit:  %d bytes in %d blocks\n", total_size, total_leaks);
    fprintf(stderr, "  Total allocated:  %d allocs, %d frees, %d bytes\n", g_stat->total_allocs, g_stat->total_free, g_stat->total_size);

    g_debug = 1;
}

struct stackframe {
  struct stackframe* ebp;
  uint32_t eip;
};

static void set_trace(int index) {
    struct stackframe *ebp;
    asm volatile("movl %%ebp, %0" : "=r" (ebp));
    ebp = ebp->ebp;

    for (int i = 0; i < 4; i++) {
        if (ebp == NULL) {
            for (int j = i; j < 4; j++)
                g_stat->allocs[index].caller[j] = NULL;
            break;
        }
        g_stat->allocs[index].caller[i] = (void *) ebp->eip;
        serial_debug("set_trace: %d %p\n", i, g_stat->allocs[index].caller[i]);
        ebp = ebp->ebp;
    }
}

static void register_alloc(void *ptr, uint32_t size) {
    serial_debug("register_alloc: %p %d\n", ptr, size);

    g_stat->total_allocs++;
    g_stat->total_size += size;

    for (int i = 0; i < g_stat->tab_size; i++) {
        if (g_stat->allocs[i].ptr)
            continue;
        g_stat->allocs[i].size = size;
        g_stat->allocs[i].ptr = ptr;
        set_trace(i);
        return;
    }

    serial_debug("register_alloc: realloc...\n", g_stat->tab_size);
    g_debug = 0;
    g_stat->allocs = realloc(g_stat->allocs, g_stat->tab_size * 2 * sizeof(leaks_stat_t));
    g_debug = 1;

    for (int i = g_stat->tab_size; i < g_stat->tab_size * 2; i++)
        g_stat->allocs[i].ptr = NULL;

    g_stat->allocs[g_stat->tab_size].size = size;
    g_stat->allocs[g_stat->tab_size].ptr = ptr;

    set_trace(g_stat->tab_size);

    g_stat->tab_size *= 2;
}

static void register_free(void *ptr) {
    serial_debug("register_free: %p\n", ptr);
    g_stat->total_free++;

    for (int i = 0; i < g_stat->tab_size; i++) {
        if (g_stat->allocs[i].ptr == ptr) {
            g_stat->allocs[i].ptr = NULL;
            return;
        }
    }

    serial_debug("register_free: not found\n");
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

#define ALLOC_DO(ptr, size)             \
    if (ptr) {                          \
        if (g_debug)                    \
            register_alloc(ptr, size);  \
        return ptr;                     \
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
    if (g_debug && mem)
        register_free(mem);

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
            if (g_debug)
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
