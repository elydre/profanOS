#include <driver/screen.h>
#include <function.h>
#include <iolib.h>
#include <string.h>
#include <mem.h>

void memory_copy(uint8_t *source, uint8_t *dest, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        *(dest + i) = *(source + i);
    }
}

void memory_set(uint8_t *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

// elydre b3 memory manager with alloc and free functions
// https://github.com/elydre/elydre/blob/main/projet/profan-tools/b3.py

#define PART_SIZE 0x1000  // 4Ko
#define IMM_COUNT 54      // can save 4104Ko
#define BASE_ADDR 0x20000

static int MLIST[IMM_COUNT];

int get_state(int imm, int index) {
    int last = -1;
    for(int i = 0; i < index + 1; i++) {
        if (last != -1) last = (last - last % 3) / 3;
        else last = imm;
    }
    return last % 3;
}

int get_required_part(int size) {
    return (size + PART_SIZE - 1) / PART_SIZE;
}

int set_state(int imm, int index, int new) {
    int old = get_state(imm, index) * pow(3, index);
    return imm - old + new * pow(3, index);
}

int alloc(int size) {
    int required_part = get_required_part(size);
    int suite = 0, num, debut, imm_debut, val;
    for (int mi = 0; mi < IMM_COUNT; mi++) {
        for (int i = 0; i < 19; i++) {
            num = get_state(MLIST[mi], i);
            if (num == 0) suite += 1;
            else suite = 0;

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
                if (k == debut) val = 1;
                else val = 2;
                MLIST[imm_debut + k / 19] = set_state(MLIST[imm_debut + k / 19], k % 19, val);
            }
            return (imm_debut * 19 + debut) * PART_SIZE + BASE_ADDR;
        }
    }
    return -1;
}

void memory_print() {
    int color, val;
    char nb[2];
    for (int mi = 0; mi < IMM_COUNT; mi++) {
        if (mi % 3 == 0) kprint("\n  ");
        kprint("    ");
        for (int i = 0; i < 19; i++) {
            val = get_state(MLIST[mi], i);
            if (val == 0) kprint("0");
            if (val == 1) color = ((i + mi) % 6 + 9) * 16;
            if (val > 0) {
                int_to_ascii(val, nb);
                ckprint(nb, color);
            }
        }
    }
    kprint("\n");
}

int free(int addr) { 
    int index = (addr - BASE_ADDR) / PART_SIZE;
    int list_index = index / 19, i = index % 19;
    if (get_state(MLIST[list_index], i) == 1) {
        MLIST[list_index + i / 19] = set_state(MLIST[list_index + i / 19], i % 19, 0);
        i++;
        while (get_state(MLIST[list_index + i / 19], i % 19) == 2) {
            MLIST[list_index + i / 19] = set_state(MLIST[list_index + i / 19], i % 19, 0);
            i++;
        } return 1;
    } return 0;
}

void * malloc(int size) {
    int addr = alloc(size);
    if (addr == -1) return NULL;
    return (void *)addr;
}

// memory info function

int get_memory_usage() {
    int used = 0;
    for (int mi = 0; mi < IMM_COUNT; mi++) {
        for (int i = 0; i < 19; i++) {
            if (get_state(MLIST[mi], i) > 0) used++;
        }
    }
    return used * (PART_SIZE / 1024);
}

int get_usable_memory() {
    return IMM_COUNT * 19 * (PART_SIZE / 1024);
}
