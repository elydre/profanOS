#include <libc/task.h>
#include <function.h>
#include <system.h>
#include <string.h>
#include <iolib.h>
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

// rip b3 (sep 2022 - nov 2022)

allocated_part_t *MEM_PARTS;
int first_part_index = 0;
int FAA; // first allocable address

// static int alloc_count = 0;
// static int free_count = 0;

void mem_init() {
    MEM_PARTS = (allocated_part_t *) BASE_ADDR;
    for (int i = 0; i < PARTS_COUNT; i++) {
        MEM_PARTS[i].state = 0;
    }

    FAA = BASE_ADDR + sizeof(allocated_part_t) * PARTS_COUNT;

    MEM_PARTS[0].state = 1;
    MEM_PARTS[0].size = 0;
    MEM_PARTS[0].addr = FAA;
    MEM_PARTS[0].next = 1;

    fskprint("memory manager initialized at %x\n", MEM_PARTS);
    fskprint("first available address: %x\n", FAA);
    fskprint("memory manager size: %dKo\n", sizeof(allocated_part_t) * PARTS_COUNT / 1024);
    mem_alloc(0x1000);
}

int mm_get_unused_index() {
    for (int i = 0; i < PARTS_COUNT; i++) {
        if (MEM_PARTS[i].state == 0) return i;
    }
    fskprint("no more block available\n");
    return -1;
}

void del_occurence(int index) {
    int i = first_part_index;
    while (MEM_PARTS[i].state != 0) {
        if (MEM_PARTS[i].next == index) {
            MEM_PARTS[i].next = mm_get_unused_index();
            return;
        }
        i = MEM_PARTS[i].next;
    }
}

uint32_t mem_alloc(uint32_t size) {
    if (size == 0) return 0;
    // parcours de la liste des parties allouées
    int index, old_index, exit_mode;
    uint32_t last_addr;
    index = first_part_index;

    last_addr = FAA;
    while (1) {
        // fskprint("$Eindex: %d, addr: %x, size: %d, state: %d, next: %d\n", index, MEM_PARTS[index].addr, MEM_PARTS[index].size, MEM_PARTS[index].state, MEM_PARTS[index].next);
        // si la partie est libre
        if (MEM_PARTS[index].state == 0) {
            // TODO: verifier si 'last_addr + size' est dans la memoire physique
            exit_mode = 1;
            break;
        }
        if (MEM_PARTS[index].addr - last_addr >= size) {
            // on peut allouer la partie ici
            exit_mode = 0;
            break;
        }

        last_addr = MEM_PARTS[index].addr + MEM_PARTS[index].size;

        old_index = index;
        index = MEM_PARTS[index].next;
    }

    // fskprint("exit mode: %d\n", exit_mode);

    int new_index = mm_get_unused_index();
    if (new_index == -1) return NULL;

    int i = exit_mode ? index: new_index;

    MEM_PARTS[i].addr = last_addr;
    MEM_PARTS[i].size = size;
    MEM_PARTS[i].task_id = task_get_current_pid();
    MEM_PARTS[i].state = 1;

    if (exit_mode == 0) {
        del_occurence(new_index);
        // fskprint("del occurence(%d)\n", new_index);

        MEM_PARTS[old_index].next = new_index;
        MEM_PARTS[new_index].next = index;
    } else {
        int new_index = mm_get_unused_index();
        if (new_index == -1) return NULL;
        MEM_PARTS[index].next = new_index;
    }

    // fskprint("new index: %d, addr: %x, size: %d, state: %d, next: %d\n", i, MEM_PARTS[i].addr, MEM_PARTS[i].size, MEM_PARTS[i].state, MEM_PARTS[i].next);

    return last_addr;
}

int mem_free_addr(uint32_t addr) {
    int index = first_part_index;
    int last_index = -1;
    while (1) {
        if (MEM_PARTS[index].addr == addr && last_index != -1) {
            MEM_PARTS[last_index].next = MEM_PARTS[index].next;
            // fskprint("clearing index %d, block %d point to %d\n", index, last_index, MEM_PARTS[last_index].next);
            MEM_PARTS[index].state = 0;
            return 1; // success
        }
        last_index = index;
        index = MEM_PARTS[index].next;
    }
    fskprint("no block found at %x\n", addr);
    return 0; // error
}

uint32_t mem_get_alloc_size(uint32_t addr) {
    uint32_t index = first_part_index;
    while (MEM_PARTS[index].state) {
        if (MEM_PARTS[index].addr == addr) {
            return MEM_PARTS[index].size;
        }
        index = MEM_PARTS[index].next;
    }
    fskprint("no block found at %x\n", addr);
    while (1);
    return 0;
}

// standard functions

void free(void *addr) {
    mem_set((uint8_t *) addr, 0, mem_get_alloc_size((uint32_t) addr));
    mem_free_addr((int) addr);
}

void *malloc(uint32_t size) {
    uint32_t addr = mem_alloc(size);
    if (addr == 0) return NULL; // error
    return (void *) addr;
}

void *realloc(void *ptr, uint32_t size) {
    uint32_t addr = (uint32_t) ptr;
    uint32_t new_addr = mem_alloc(size);
    if (new_addr == 0) return NULL;
    mem_copy((uint8_t *) addr, (uint8_t *) new_addr, size);
    mem_free_addr(addr);
    return (void *) new_addr;
}

void *calloc(uint32_t size) {
    int addr = mem_alloc(size);
    if (addr == 0) return NULL;
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
    int index = first_part_index;
    while (MEM_PARTS[index].state) {
        fskprint("index: %d, addr: %x, size: %d, state: %d, next: %d\n", index, MEM_PARTS[index].addr, MEM_PARTS[index].size, MEM_PARTS[index].state, MEM_PARTS[index].next);
        index = MEM_PARTS[index].next;
    }
}

int mem_get_usage() {
    // TODO: implement
    return 0;
}

int mem_get_usable() {
    // TODO: implement
    return 0;
}

int mem_get_alloc_count() {
    // TODO: implement
    return 0;
}

int mem_get_free_count() {
    // TODO: implement
    return 0;
}

int mem_get_base_addr() {
    return (int) FAA;
}
