/*****************************************************************************\
|   === malloc.c : 2024 ===                                                   |
|                                                                             |
|    Implementation of malloc functions and leak tracking          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan.h>
#include <stdlib.h>

#undef realloc
#undef calloc
#undef malloc
#undef free

#include "config_libc.h"

#define BUDDY_ALLOC_IMPLEMENTATION
#include "alloc_buddy.h"
#undef BUDDY_ALLOC_IMPLEMENTATION

typedef struct alloc_debug {
    void *ptr;
    uint32_t size;
    void *caller[4];
} alloc_debug_t;

typedef struct {
    struct alloc_debug *allocs;
    uint32_t no_free_before;
    uint32_t total_allocs;
    uint32_t total_free;
    uint32_t total_size;
    uint32_t tab_size;
} leaks_stat_t;

leaks_stat_t *g_stat;
struct buddy  *g_buddy;

uint32_t g_arena_count;  // in pages (4KB)
uint32_t g_mdata_count;  // in pages (4KB)

int      g_debug;

static void buddy_show_leaks(void);

static void put_error(char *str) {
    fm_write(2, str, strlen(str));
}

void __buddy_init(void) {
    g_stat = NULL;
    g_debug = 0;

    g_arena_count = _BUDDY_DEFAULT_SIZE;
    if (!syscall_scuba_generate(_BUDDY_ARENA_ADDR, g_arena_count)) {
        put_error("libc: init buddy: page allocation failed\n");
        return;
    }

    g_mdata_count = buddy_sizeof(g_arena_count * 4096) / 4096 + 1;
    if (!syscall_scuba_generate(_BUDDY_MDATA_ADDR, g_mdata_count)) {
        put_error("libc: init buddy: page allocation failed\n");
        return;
    }

    g_buddy = buddy_init(_BUDDY_MDATA_ADDR, _BUDDY_ARENA_ADDR, g_arena_count * 4096);

    if (g_buddy == NULL)
        put_error("libc: init buddy: buddy_init failed\n");
}

void __buddy_enable_leaks(void) {
    if (g_stat)
        return;
    g_stat = calloc(1, sizeof(leaks_stat_t));
    g_stat->allocs = calloc(1, sizeof(leaks_stat_t));
    g_stat->tab_size = 1;
    g_debug = 1;
}

void __buddy_disable_leaks(void) {
    if (!g_stat)
        return;
    buddy_show_leaks();
    g_debug = 0;

    free(g_stat->allocs);
    free(g_stat);
    g_stat = NULL;
}

static void buddy_show_leaks(void) {
    if (!g_debug)
        return;
    g_debug = 0;

    uint32_t total_leaks = 0;
    uint32_t total_size = 0;
    char *libname;

    for (uint32_t i = 0; i < g_stat->tab_size; i++) {
        if (!g_stat->allocs[i].ptr)
            continue;
        total_size += g_stat->allocs[i].size;
        total_leaks++;

        if (total_leaks == 1)
            fputs("===================  NOT  FREED  AT  EXIT  ===================\n\n", stderr);

        if (total_leaks == 1000)
            fputs("Only the first 1000 leaks are shown\n", stderr);

        if (total_leaks >= 1000)
            continue;

        fprintf(stderr, "%d bytes at %p\n", g_stat->allocs[i].size, g_stat->allocs[i].ptr);
        for (int j = 0; j < 4; j++) {
            void *ptr = g_stat->allocs[i].caller[j];
            if (ptr == NULL)
                break;
            char *name = profan_fn_name(ptr, &libname);
            fprintf(stderr, "  0x%08x %s (%s)\n", ptr, name ? name : "???", libname ? libname : "???");
        }
        fputs("  ...\n\n", stderr);
    }

    fputs("\n==================  MEMORY  USAGE  SUMMARY  ==================\n\n", stderr);
    fprintf(stderr, "  Unfreed at exit:  %d bytes in %d blocks\n", total_size, total_leaks);
    fprintf(stderr, "  Total allocated:  %d allocs, %d frees, %d bytes\n",
            g_stat->total_allocs, g_stat->total_free, g_stat->total_size);
    fputs("\n==============================================================\n", stderr);

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
        ebp = ebp->ebp;
    }
}

static void register_alloc(void *ptr, size_t size) {
    g_stat->total_allocs++;
    g_stat->total_size += size;

    for (uint32_t i = g_stat->no_free_before; i < g_stat->tab_size; i++) {
        if (g_stat->allocs[i].ptr)
            continue;
        g_stat->no_free_before = i + 1;
        g_stat->allocs[i].size = size;
        g_stat->allocs[i].ptr = ptr;
        set_trace(i);
        return;
    }

    g_debug = 0;
    alloc_debug_t *new = malloc(g_stat->tab_size * 2 * sizeof(alloc_debug_t));

    if (new == NULL) {
        g_debug = 1;
        return;
    }

    memcpy(new, g_stat->allocs, g_stat->tab_size * sizeof(alloc_debug_t));
    free(g_stat->allocs);

    g_debug = 1;

    g_stat->allocs = new;

    for (uint32_t i = g_stat->tab_size; i < g_stat->tab_size * 2; i++)
        g_stat->allocs[i].ptr = NULL;

    g_stat->allocs[g_stat->tab_size].size = size;
    g_stat->allocs[g_stat->tab_size].ptr = ptr;

    set_trace(g_stat->tab_size);

    g_stat->tab_size *= 2;
}

static void register_free(void *ptr) {
    g_stat->total_free++;

    for (uint32_t i = 0; i < g_stat->tab_size; i++) {
        if (g_stat->allocs[i].ptr != ptr)
            continue;
        if (g_stat->no_free_before > i)
            g_stat->no_free_before = i;
        g_stat->allocs[i].ptr = NULL;
        return;
    }

    put_error("libc: leak tracking: internal error\n");
}

static int extend_virtual(size_t size) {
    uint32_t req = g_arena_count + size / 2048 + 1;

    // align to the next power of 2 (11 -> 16)
    while (req & (req - 1))
        req += req & -req;

    // grow the metadata if needed
    uint32_t mdata_count = buddy_sizeof(req * 4096) / 4096 + 1;
    if (mdata_count > g_mdata_count) {
        mdata_count += 2; // avoid too many small resizes
        if (mdata_count > (_BUDDY_ARENA_ADDR - _BUDDY_MDATA_ADDR) / 4096) {
            put_error("libc: extend_virtual: metadata overflow\n");
            return 1;
        }
        if (!syscall_scuba_generate(_BUDDY_MDATA_ADDR + g_mdata_count * 4096, mdata_count - g_mdata_count))
            return 1;
        g_mdata_count = mdata_count;
    }

    if (!syscall_scuba_generate(_BUDDY_ARENA_ADDR + g_arena_count * 4096, req - g_arena_count))
        return 1;

    g_arena_count = req;

    g_buddy = buddy_resize(g_buddy, g_arena_count * 4096);

    if (g_buddy == NULL) {
        put_error("libc: extend_virtual: buddy_resize failed\n");
        return 1;
    }

    return 0;
}

#define ALLOC_DO(ptr, size)             \
    if (ptr) {                          \
        if (g_debug)                    \
            register_alloc(ptr, size);  \
        return ptr;                     \
    }

void *malloc(size_t size) {
    void *p = buddy_malloc(g_buddy, size);
    ALLOC_DO(p, size);

    extend_virtual(size);

    p = buddy_malloc(g_buddy, size);
    ALLOC_DO(p, size);

    put_error("libc: malloc: not enough memory\n");
    return NULL;
}

void *calloc(size_t nmemb, size_t lsize) {
    void *p = buddy_calloc(g_buddy, nmemb, lsize);
    ALLOC_DO(p, nmemb * lsize);

    extend_virtual(nmemb * lsize);

    p = buddy_calloc(g_buddy, nmemb, lsize);
    ALLOC_DO(p, nmemb * lsize);

    put_error("libc: calloc: not enough memory\n");
    return NULL;
}

void *realloc(void *mem, size_t new_size) {
    if (g_debug && mem)
        register_free(mem);

    void *p = buddy_realloc(g_buddy, mem, new_size, 0);
    ALLOC_DO(p, new_size);

    if (new_size == 0)
        return NULL;

    extend_virtual(new_size);

    p = buddy_realloc(g_buddy, mem, new_size, 0);
    ALLOC_DO(p, new_size);

    put_error("libc: realloc: not enough memory\n");
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
