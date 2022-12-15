#include <gui/gnrtx.h>
#include <function.h>
#include <system.h>
#include <string.h>
#include <mem.h>

void mem_copy(uint8_t *source, uint8_t *dest, int nbytes) {
    for (int i = 0; i < nbytes; i++) *(dest + i) = *(source + i);
}

void mem_set(uint8_t *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

void mem_move(uint8_t *source, uint8_t *dest, int nbytes) {
    if (source < dest) {
        for (int i = nbytes - 1; i >= 0; i--) {
            *(dest + i) = *(source + i);
        }
    } else {
        for (int i = 0; i < nbytes; i++) {
            *(dest + i) = *(source + i);
        }
    }
}

// elydre b3 memory manager with mem_alloc and free_addr functions
// https://github.com/elydre/elydre/blob/main/projet/profan/b3.py

#define PART_SIZE 0x1000   // 4Ko
#define IMM_COUNT 108      // can save ~8Mo
#define BASE_ADDR 0x250000 // lot of jokes here

static int MLIST[IMM_COUNT];
static int alloc_count = 0;
static int free_count = 0;

int get_state(int imm, int index) {
    int last = -1;
    for (int i = 0; i < index + 1; i++) {
        last = (last != -1) ? (last - last % 3) / 3 : imm;
    }
    return last % 3;
}

int get_required_part(int size) {
    if (size < 1) size = 1;
    return (size + PART_SIZE - 1) / PART_SIZE;
}

int set_state(int imm, int index, int new) {
    int old = get_state(imm, index) * pow(3, index);
    return imm - old + new * pow(3, index);
}

int mem_alloc(int size) {
    int required_part = get_required_part(size);
    int suite = 0, num, debut, imm_debut, val;
    for (int mi = 0; mi < IMM_COUNT; mi++) {
        for (int i = 0; i < 19; i++) {
            num = get_state(MLIST[mi], i);
            suite = (num == 0) ? suite + 1 : 0;

            if (!(suite == required_part)) continue;
            debut = i - required_part + 1;

            if (debut < 0) {
                imm_debut = (-debut) / 19 + 1;
                debut = 19 * imm_debut + debut;
                imm_debut = mi - imm_debut;
            } else {
                imm_debut = mi;
            }

            for (int k = debut; k < debut + required_part; k++) {
                val = (k == debut) ? 1 : 2;
                MLIST[imm_debut + k / 19] = set_state(MLIST[imm_debut + k / 19], k % 19, val);
            }
            alloc_count++;
            return (imm_debut * 19 + debut) * PART_SIZE + BASE_ADDR;
        }
    }
    sys_warning("memory allocation failed");
    return -1;
}

int mem_free_addr(int addr) {
    int index = (addr - BASE_ADDR) / PART_SIZE;
    int list_index = index / 19, i = index % 19;
    if (get_state(MLIST[list_index], i) == 1) {
        MLIST[list_index + i / 19] = set_state(MLIST[list_index + i / 19], i % 19, 0);
        i++;
        while (get_state(MLIST[list_index + i / 19], i % 19) == 2) {
            MLIST[list_index + i / 19] = set_state(MLIST[list_index + i / 19], i % 19, 0);
            i++;
        }
        free_count++;
        return 1;
    } return 0;
}

int mem_get_alloc_size(int addr) {
    int index = (addr - BASE_ADDR) / PART_SIZE;
    int list_index = index / 19, i = index % 19;
    if (get_state(MLIST[list_index], i) == 1) {
        int size = 0;
        while (get_state(MLIST[list_index + i / 19], i % 19) == 2) {
            size += PART_SIZE;
            i++;
        } return size + PART_SIZE;
    } return 0;
}

// standard functions

void free(void *addr) {
    mem_set((uint8_t *) addr, 0, mem_get_alloc_size((int) addr));
    mem_free_addr((int) addr);
}

void *malloc(int size) {
    int addr = mem_alloc(size);
    if (addr == -1) return NULL;
    return (void *) addr;
}

void *realloc(void *ptr, int size) {
    int addr = (int) ptr;
    int new_addr = mem_alloc(size);
    if (new_addr == -1) return NULL;
    mem_copy((uint8_t *) addr, (uint8_t *) new_addr, size);
    mem_free_addr(addr);
    return (void *) new_addr;
}

void *calloc(int size) {
    int addr = mem_alloc(size);
    if (addr == -1) return NULL;
    mem_set((uint8_t *) addr, 0, size);
    return (void *) addr;
}

// memory info function

int mem_get_phys_size() {
    int *addr_min = (int *) 0x200000;
    int *addr_max = (int *) 0x40000000;
    int *addr_test;
    while (addr_max - addr_min > 1) {
        addr_test = addr_min + (addr_max - addr_min) / 2;
        * addr_test = 0x1234;
        if (*addr_test == 0x1234) addr_min = addr_test;
        else addr_max = addr_test;
    }
    return (int) addr_max;
}

void mem_print() {
    int val, color = 0x80;
    char nb[2];
    int nb_lines = (IMM_COUNT > (gt_get_max_rows() - 2) *3 ? (gt_get_max_rows() - 2) *3 : IMM_COUNT);
    for (int mi = 0; mi < nb_lines; mi++) {
        if (mi % 3 == 0) kprint("\n  ");
        kprint("    ");
        for (int i = 0; i < 19; i++) {
            val = get_state(MLIST[mi], i);
            if (val == 0) kprint("0");
            if (val == 1) color = (color > 0xE0) ? 0x80 : color + 0x10;
            if (val > 0) {
                int_to_ascii(val, nb);
                ckprint(nb, color);
            }
        }
    }
    kprint((IMM_COUNT > nb_lines) ? "\n      ...\n" : "\n\n");
}

int mem_get_usage() {
    int used = 0;
    for (int mi = 0; mi < IMM_COUNT; mi++) {
        for (int i = 0; i < 19; i++) {
            if (get_state(MLIST[mi], i) > 0) used++;
        }
    }
    return used * (PART_SIZE / 1024);
}

int mem_get_usable() {
    return IMM_COUNT * 19 * (PART_SIZE / 1024);
}

int mem_get_alloc_count() {
    return alloc_count;
}

int mem_get_free_count() {
    return free_count;
}

int mem_get_base_addr() {
    return BASE_ADDR;
}
