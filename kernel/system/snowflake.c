/*****************************************************************************\
|   === snowflake.c : 2024 ===                                                |
|                                                                             |
|    Kernel physical memory allocator                              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    rip b3 (september - december 2022)                            `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/snowflake.h>
#include <kernel/multiboot.h>
#include <drivers/diskiso.h>
#include <kernel/process.h>
#include <minilib.h>
#include <system.h>


allocated_part_t *MEM_PARTS;
uint32_t alloc_count;
uint32_t free_count;
uint32_t phys_size;
int instance_count;
int part_size;

static void *mem_alloc_func(uint32_t size,
        int state, uint32_t allign, int pid, void *dir);

static uint32_t mem_get_phys_size(void) {
    uint32_t *addr_min, *addr_max;
    uint32_t *addr_test, old_value;

    addr_test = (uint32_t *) (g_mboot->mem_upper * 1024);

    if (addr_test)
        return (uint32_t) addr_test;

    // if the multiboot info is not available
    addr_min = (uint32_t *) 0x200000;
    addr_max = (uint32_t *) 0x40000000;
    while (addr_max - addr_min > 1) {
        addr_test = addr_min + (addr_max - addr_min) / 2;
        old_value = *addr_test;
        *addr_test = 0x1234;
        if (*addr_test == 0x1234) {
            addr_min = addr_test;
            *addr_test = old_value;
        }
        else addr_max = addr_test;
    }
    return (uint32_t) addr_max;
}


int mem_init(void) {
    alloc_count = 0;
    free_count = 0;

    instance_count = 0;

    phys_size = mem_get_phys_size();
    if (phys_size > SCUBA_MAP_MAX) {
        phys_size = SCUBA_MAP_MAX;
    }

    part_size = PARTS_COUNT;

    MEM_PARTS = (allocated_part_t *) (MEM_BASE_ADDR + 1);

    MEM_PARTS[0].state = SNOW_BASE;
    MEM_PARTS[0].size  = 1;
    MEM_PARTS[0].addr  = MEM_BASE_ADDR;
    MEM_PARTS[0].next  = 1;
    MEM_PARTS[0].pid   = 0;
    MEM_PARTS[0].dir   = NULL;

    for (int i = 1; i < part_size; i++)
        MEM_PARTS[i].state = SNOW_FREE;

    if (mem_alloc(sizeof(allocated_part_t) * part_size, SNOW_MM, 0) != MEM_PARTS)
        sys_fatal("Snowflake address is illogical");

    // allocate the diskiso module if needed
    if (!diskiso_get_size())
        return 0;

    void *start = mem_alloc(diskiso_get_size(), SNOW_SIMPLE, 0);

    if (start != diskiso_get_start()) {
        sys_fatal("Diskiso address is illogical (%x != %x)", start, diskiso_get_start());
    }

    return 0;
}


static int get_unused_index(int not_index) {
    for (int i = 0; i < part_size; i++)
        if (MEM_PARTS[i].state == SNOW_FREE && i != not_index)
            return i;

    sys_fatal("No more memory, dynamizing do not work");
    return -1;
}


static void del_occurence(uint32_t index) {
    int i = 0;

    for (int count = 0; MEM_PARTS[i].state; count++) {
        if (MEM_PARTS[i].next == index) {
            MEM_PARTS[i].next = get_unused_index(index);
            return;
        }

        if (count > part_size)
            sys_fatal("Recursive linked list detected in snowflake");

        i = MEM_PARTS[i].next;
    }
}


static void dynamize_mem(void) {
    int sum = 0;

    for (int i = 0; i < part_size; i++) {
        sum += MEM_PARTS[i].state == SNOW_FREE;
        if (sum > 4)
            return;
    }

    void *new_add = mem_alloc_func(sizeof(allocated_part_t) * (part_size + GROW_SIZE), SNOW_MM, 0, 0, NULL);

    if (new_add == NULL) {
        sys_fatal("Memory dynamizing failed");
        return;
    }

    // fill the new part with 0 and copy the old part
    mem_set(new_add, 0, sizeof(allocated_part_t) * (part_size + GROW_SIZE));
    mem_copy(new_add, MEM_PARTS, sizeof(allocated_part_t) * part_size);

    void *old_addr = MEM_PARTS;
    MEM_PARTS = new_add;
    part_size += GROW_SIZE;

    mem_free_addr(old_addr);
}

static void *mem_alloc_func(uint32_t size, int state, uint32_t allign, int pid, void *dir) {
    if (!(size && state))
        return 0;

    if (state != SNOW_MM) {
        dynamize_mem();
        instance_count++;
    }

    if (instance_count > 1) {
        sys_fatal("[mem_alloc] Instance count is too high");
    }

    // traversing the list of allocated parts
    int index, old_index, exit_mode;
    uint32_t last_addr, gap;
    index = 0;

    last_addr = MEM_BASE_ADDR;

    if (MEM_PARTS[0].state != SNOW_BASE) {
        sys_fatal("[mem_alloc] Snowflake is corrupted");
    }

    for (int i = 0; i <= part_size; i++) {
        if (i == part_size) {
            sys_fatal("Recursive linked list detected in snowflake");
        }

        // calculate the gap
        gap = allign ? (allign - (last_addr % allign)) % allign : 0;

        // if the part is free
        if (MEM_PARTS[index].state == SNOW_FREE) {
            exit_mode = 1 + (last_addr + size + gap > phys_size);
            break;
        }

        if (MEM_PARTS[index].addr - last_addr >= size + gap && last_addr != MEM_BASE_ADDR) {
            // we can allocate the part here
            exit_mode = 0;
            break;
        }

        last_addr = MEM_PARTS[index].addr + MEM_PARTS[index].size;

        old_index = index;
        index = MEM_PARTS[index].next;
    }

    if (exit_mode == 2) {
        if (state != SNOW_MM)
            instance_count--;
        sys_error("[mem_alloc] %dk of memory is not available", size / 1024);
        return 0;
    }

    int new_index;

    if (exit_mode) {
        new_index = index;
        MEM_PARTS[new_index].state = state;
        MEM_PARTS[index].next = get_unused_index(-1);
    } else {
        new_index = get_unused_index(-1);
        del_occurence(new_index);
        MEM_PARTS[old_index].next = new_index;
        MEM_PARTS[new_index].next = index;
        MEM_PARTS[new_index].state = state;
    }

    MEM_PARTS[new_index].addr  = last_addr + gap;
    MEM_PARTS[new_index].size  = size;
    MEM_PARTS[new_index].pid   = pid;
    MEM_PARTS[new_index].dir   = dir;

    alloc_count++;

    if (state != SNOW_MM)
        instance_count--;

    return (void *) last_addr + gap;
}


void *mem_alloc(uint32_t size, int state, uint32_t allign) {
    int pid;

    if (state == SNOW_BASE || state == SNOW_SCUBA) {
        sys_warning("[mem_alloc] state %d is reserved", state);
        return NULL;
    }

    if (state == SNOW_SIMPLE) {
        pid = process_get_pid();
    } else if (state == SNOW_ARGS) {
        pid = allign ? allign : process_get_pid();
        allign = 0; // allign is not used in SNOW_ARGS state
    } else {
        pid = 0;
    }

    return mem_alloc_func(size, state, allign, pid, NULL);
}


void *mem_alloc_dir(uint32_t size, uint32_t allign, int pid, void *dir) {
    return mem_alloc_func(size, SNOW_SCUBA, allign, pid, dir);
}


int mem_free_addr(void *addr) {
    instance_count++;

    if (instance_count > 1) {
        sys_fatal("[mem_free] Instance count is too high");
    }

    int last_index = -1;
    int index = 0;

    while (MEM_PARTS[index].state != SNOW_FREE) {
        if (!(MEM_PARTS[index].addr == (uint32_t) addr && last_index != -1)) {
            last_index = index;
            index = MEM_PARTS[index].next;
            continue;
        }

        if (MEM_PARTS[index].state == SNOW_BASE) {
            instance_count--;
            sys_warning("Cannot free snowflake memory");
            return 1;
        }

        free_count++;
        MEM_PARTS[last_index].next = MEM_PARTS[index].next;
        MEM_PARTS[index].state = SNOW_FREE;
        instance_count--;
        return 0;
    }

    instance_count--;
    sys_warning("Cannot free memory at address %x", addr);

    return 1;
}


int mem_alloc_fetch(void *addr, int arg) {
    uint32_t index = 0;
    int state;

    while (MEM_PARTS[index].state != SNOW_FREE) {
        if (MEM_PARTS[index].addr != (uint32_t) addr) {
            index = MEM_PARTS[index].next;
            continue;
        }

        if (arg == 0)
            return MEM_PARTS[index].size;

        state = MEM_PARTS[index].state;
        if (state != SNOW_SIMPLE && state != SNOW_ARGS) {
            sys_warning("[mem_alloc_fetch] state %d is pid independent", state);
            return -1;
        }

        MEM_PARTS[index].pid = arg;
        return 0;
    }

    return -1; // block not found
}


int mem_free_all(int pid, int state) {
    uint32_t index = 0;
    int count = 0;

    if (pid == 0) {
        sys_warning("[mem_free_all] Cannot free memory of kernel");
        return -1;
    }

    while (MEM_PARTS[index].state) {
        if (MEM_PARTS[index].pid == pid &&
                ((!state && (MEM_PARTS[index].state != SNOW_SCUBA)) || MEM_PARTS[index].state == state))
            count += !mem_free_addr((void *) MEM_PARTS[index].addr);
        index = MEM_PARTS[index].next;
    }

    return count;
}


int mem_free_dir(void *dir) {
    uint32_t index = 0;
    int count = 0;

    while (MEM_PARTS[index].state) {
        if (MEM_PARTS[index].dir == dir)
            count += !mem_free_addr((void *) MEM_PARTS[index].addr);
        index = MEM_PARTS[index].next;
    }

    return count;
}

uint32_t mem_get_info(char get_mode, int get_arg) {
    uint32_t count = 0;
    uint32_t size = 0;
    uint32_t index = 0;

    switch (get_mode) {
        case 0:
            return phys_size;
        case 1:
            return MEM_BASE_ADDR;
        case 2:
            return sizeof(allocated_part_t) * part_size;
        case 3:
            return (int) MEM_PARTS;
        case 4:
            return alloc_count;
        case 5:
            return free_count;
        case 6:
            while (MEM_PARTS[index].state != SNOW_FREE) {
                count += MEM_PARTS[index].size;
                index = MEM_PARTS[index].next;
            }
            return count;
        case 7:
        case 8:
            while (MEM_PARTS[index].state != SNOW_FREE) {
                if (MEM_PARTS[index].pid == get_arg && MEM_PARTS[index].state == SNOW_SIMPLE) {
                    size += MEM_PARTS[index].size;
                    count++;
                }
                index = MEM_PARTS[index].next;
            }
            return get_mode == 7 ? count : size;
        case 9:
        case 10:
            while (MEM_PARTS[index].state) {
                if (MEM_PARTS[index].state == get_arg) {
                    size += MEM_PARTS[index].size;
                    count++;
                }
                index = MEM_PARTS[index].next;
            }
            return get_mode == 9 ? count : size;
        case 11:
        case 12:
            while (MEM_PARTS[index].state) {
                if (MEM_PARTS[index].pid == get_arg) {
                    size += MEM_PARTS[index].size;
                    count++;
                }
                index = MEM_PARTS[index].next;
            }
            return get_mode == 11 ? count : size;
        default:
            break;
    }

    return -1;
}
