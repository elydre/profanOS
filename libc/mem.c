#include <driver/serial.h>
#include <libc/task.h>
#include <system.h>
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

/*********************************
 * rip b3 (sep 2022 - dec 2022) *
*********************************/

allocated_part_t *MEM_PARTS;
uint32_t mm_struct_addr;
int first_part_index;
int part_size;

// static int alloc_count = 0;
// static int free_count = 0;

uint32_t mem_alloc(uint32_t size, int state);
int mem_free_addr(uint32_t addr);

void mem_init() {
    first_part_index = 0;
    part_size = PARTS_COUNT;

    MEM_PARTS = (allocated_part_t *) (MEM_BASE_ADDR + 1);
    for (int i = 0; i < part_size; i++) {
        MEM_PARTS[i].state = 0;
    }

    MEM_PARTS[0].state = 2;
    MEM_PARTS[0].size = 1;
    MEM_PARTS[0].addr = MEM_BASE_ADDR;
    MEM_PARTS[0].next = 1;

    if (mem_alloc(sizeof(allocated_part_t) * part_size, 3) != (uint32_t) MEM_PARTS) {
        sys_fatal("snowflake address is illogical");
    }

    fskprint("memory manager initialized at %x\n", MEM_PARTS);
    fskprint("memory manager size: %do\n", sizeof(allocated_part_t) * part_size);
}

int mm_get_unused_index() {
    for (int i = 0; i < part_size; i++) {
        if (MEM_PARTS[i].state == 0) return i;
    }

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

void dynamize_mem() {
    int sum = 0;
    for (int i = 0; i < part_size; i++) {
        sum += !MEM_PARTS[i].state;
        if (sum > 3) return;
    }

    serial_debug("SNOWFLAKE", "dynamizing memory...");
    uint32_t new_add = mem_alloc(sizeof(allocated_part_t) * (part_size + GROW_SIZE), 3);
    if (new_add == 0) {
        sys_fatal("memory dynamizing failed");
        return;
    }
    mem_copy((uint8_t *) MEM_PARTS, (uint8_t *) new_add, sizeof(allocated_part_t) * part_size);
    uint32_t old_add = (uint32_t) MEM_PARTS;
    MEM_PARTS = (allocated_part_t *) new_add;
    part_size += GROW_SIZE;
    mem_free_addr(old_add);

    serial_debug("SNOWFLAKE", "memory successfully dynamized");
}

uint32_t mem_alloc(uint32_t size, int state) {
    if (!size) return 0;
    if (!state) return 0;
    if (state != 3) dynamize_mem();

    // parcours de la liste des parties allouées
    int index, old_index, exit_mode;
    uint32_t last_addr;
    index = first_part_index;

    last_addr = MEM_BASE_ADDR;
    while (1) {
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

    int new_index = mm_get_unused_index();
    if (new_index == -1) return 0;

    int i = exit_mode ? index: new_index;

    MEM_PARTS[i].addr = last_addr;
    MEM_PARTS[i].size = size;
    MEM_PARTS[i].task_id = (state == 3) ? 0 : task_get_current_pid();
    MEM_PARTS[i].state = state;

    if (exit_mode == 0) {
        del_occurence(new_index);
        // fskprint("del occurence(%d)\n", new_index);

        MEM_PARTS[old_index].next = new_index;
        MEM_PARTS[new_index].next = index;
    } else {
        int new_index = mm_get_unused_index();
        if (new_index == -1) return 0;
        MEM_PARTS[index].next = new_index;
    }
    // fskprint("alloc %d bytes at %x\n", size, last_addr);
    return last_addr;
}

int mem_free_addr(uint32_t addr) {
    int index = first_part_index;
    int last_index = -1;
    while (1) {
        if (MEM_PARTS[index].addr == addr && last_index != -1) {
            if (MEM_PARTS[index].state == 2) {
                sys_error("cannot free first block");
                return 0; // error
            } else {
                MEM_PARTS[last_index].next = MEM_PARTS[index].next;
                MEM_PARTS[index].state = 0;
                return 1; // success
            }
        }
        last_index = index;
        index = MEM_PARTS[index].next;
    }
    sys_warning("block not found");
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
    sys_warning("block not found");
    return 0;
}

// standard functions

void free(void *addr) {
    int size = mem_get_alloc_size((uint32_t) addr);
    if (size == 0) return;
    mem_set((uint8_t *) addr, 0, size);
    mem_free_addr((int) addr);
}

void *malloc(uint32_t size) {
    uint32_t addr = mem_alloc(size, 1);
    if (addr == 0) return NULL; // error
    return (void *) addr;
}

void *realloc(void *ptr, uint32_t size) {
    uint32_t addr = (uint32_t) ptr;
    uint32_t new_addr = mem_alloc(size, 1);
    if (new_addr == 0) return NULL;
    mem_copy((uint8_t *) addr, (uint8_t *) new_addr, size);
    mem_free_addr(addr);
    return (void *) new_addr;
}

void *calloc(uint32_t size) {
    int addr = mem_alloc(size, 1);
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
        fskprint("index: %d, addr: %x, size: %d, state: %d, task: %d, next: %d\n",
                index,
                MEM_PARTS[index].addr,
                MEM_PARTS[index].size,
                MEM_PARTS[index].state,
                MEM_PARTS[index].task_id,
                MEM_PARTS[index].next
        );
        index = MEM_PARTS[index].next;
    }
}
