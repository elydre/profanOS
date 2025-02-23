/*****************************************************************************\
|   === snowflake.c : 2024 ===                                                |
|                                                                             |
|    Kernel physical memory allocator                              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/snowflake.h>
#include <kernel/multiboot.h>
#include <drivers/diskiso.h>
#include <kernel/process.h>
#include <drivers/serial.h>
#include <minilib.h>
#include <system.h>


/* * * * * * * * * * * * * * * * * * * *
 *        _             _              *
 *   ___ | | _   _   __| | _ __   ___  *
 *  / _ \| || | | | / _` || '__| / _ | *
 * |  __/| || |_| || (_| || |   |  __/ *
 *  \___||_| \__, | \__,_||_|    \___| *
 *           |___/                     *
 *                                     *
 *    rip b3  (sep 2022 - dec 2022)    *
 * * * * * * * * * * * * * * * * * * * */

allocated_part_t *MEM_PARTS;
uint32_t alloc_count;
uint32_t free_count;
uint32_t phys_size;
int part_size;

static uint32_t mem_get_phys_size(void) {
    uint32_t *addr_min, *addr_max;
    uint32_t *addr_test, old_value;

    addr_test = (uint32_t *) (mboot_get(2) * 1024);
    if (addr_test) return (uint32_t) addr_test;

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

int instance_count;

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
    for (int i = 0; i < part_size; i++) {
        MEM_PARTS[i].state = 0;
    }

    MEM_PARTS[0].state = 2;
    MEM_PARTS[0].size  = 1;
    MEM_PARTS[0].addr  = MEM_BASE_ADDR;
    MEM_PARTS[0].next  = 1;
    MEM_PARTS[0].pid   = -1;

    if (mem_alloc(sizeof(allocated_part_t) * part_size, 0, 3) != MEM_PARTS) {
        sys_fatal("Snowflake address is illogical");
    }

    // allocate the diskiso module if needed
    if (!diskiso_get_size()) return 0;

    void *start = mem_alloc(diskiso_get_size(), 0, 1);

    if (start != diskiso_get_start()) {
        sys_error("Diskiso address is illogical");
        mem_free_addr(start);
    }

    return 0;
}

static int get_unused_index(int not_index) {
    for (int i = 0; i < part_size; i++) {
        if (MEM_PARTS[i].state == 0 && i != not_index) return i;
    }

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
        if (count > part_size) {
            sys_fatal("Recursive linked list detected in snowflake");
        }
        i = MEM_PARTS[i].next;
    }
}

static void dynamize_mem(void) {
    int sum = 0;
    for (int i = 0; i < part_size; i++) {
        sum += !MEM_PARTS[i].state;
        if (sum > 4) return;
    }

    void *new_add = mem_alloc(sizeof(allocated_part_t) * (part_size + GROW_SIZE), 0, 3);

    if (new_add == 0) {
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

static void *mem_alloc_func(uint32_t size, uint32_t allign, int state, int pid, void *dir) {
    if (!(size && state)) return 0;

    if (state != 3) {
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

    if (MEM_PARTS[0].state != 2) {
        sys_fatal("[mem_alloc] Snowflake is corrupted");
    }

    for (int i = 0; i <= part_size; i++) {
        if (i == part_size) {
            sys_fatal("Recursive linked list detected in snowflake");
        }

        // calculate the gap
        gap = allign ? (allign - (last_addr % allign)) % allign : 0;

        // if the part is free
        if (MEM_PARTS[index].state == 0) {
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
        if (state != 3)
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
    if (state != 3) instance_count--;

    return (void *) last_addr + gap;
}

void *mem_alloc(uint32_t size, uint32_t allign, int state) {
    return mem_alloc_func(size, allign, state, process_get_pid(), NULL);
}

void *mem_alloc_dir(uint32_t size, uint32_t allign, int pid, void *dir) {
    return mem_alloc_func(size, allign, 4, pid, dir);
}

int mem_free_addr(void *addr) {
    instance_count++;

    if (instance_count > 1) {
        sys_fatal("[mem_free] Instance count is too high");
    }

    int index = 0;
    int last_index = -1;
    while (MEM_PARTS[index].state) {
        if (!(MEM_PARTS[index].addr == (uint32_t) addr && last_index != -1)) {
            last_index = index;
            index = MEM_PARTS[index].next;
            continue;
        }

        if (MEM_PARTS[index].state == 2) {
            instance_count--;
            sys_warning("Cannot free snowflake memory");
            return 1; // error
        }

        free_count++;
        MEM_PARTS[last_index].next = MEM_PARTS[index].next;
        MEM_PARTS[index].state = 0;
        instance_count--;
        return 0; // success
    }

    instance_count--;
    sys_warning("Cannot free memory at address %x", addr);
    return 1; // error
}

uint32_t mem_get_alloc_size(void *addr) {
    uint32_t index = 0;
    while (MEM_PARTS[index].state) {
        if (MEM_PARTS[index].addr == (uint32_t) addr) {
            return MEM_PARTS[index].size;
        }
        index = MEM_PARTS[index].next;
    }
    return 0; // block not found
}

int mem_free_all(uint32_t pid) {
    uint32_t index = 0;
    int count = 0;
    while (MEM_PARTS[index].state) {
        if (MEM_PARTS[index].pid == (int) pid && MEM_PARTS[index].state == 1) {
            count += !mem_free_addr((void *) MEM_PARTS[index].addr);
        }
        index = MEM_PARTS[index].next;
    }
    return count;
}

int mem_free_dir(void *dir) {
    uint32_t index = 0;
    int count = 0;
    while (MEM_PARTS[index].state) {
        if (MEM_PARTS[index].dir == dir) {
            count += !mem_free_addr((void *) MEM_PARTS[index].addr);
        }
        index = MEM_PARTS[index].next;
    }
    return count;
}

uint32_t mem_get_info(char get_mode, int get_arg) {
    uint32_t count = 0;
    uint32_t size = 0;
    uint32_t index = 0;

    switch (get_mode) {
        case 0: return phys_size;
        case 1: return MEM_BASE_ADDR;
        case 2: return sizeof(allocated_part_t) * part_size;
        case 3: return (int) MEM_PARTS;
        case 4: return alloc_count;
        case 5: return free_count;
        case 6:
            while (MEM_PARTS[index].state) {
                count += MEM_PARTS[index].size;
                index = MEM_PARTS[index].next;
            }
            return count;
        case 7:
        case 8:
            while (MEM_PARTS[index].state) {
                if (MEM_PARTS[index].pid == get_arg && MEM_PARTS[index].state == 1) {
                    count++;
                    size += MEM_PARTS[index].size;
                }
                index = MEM_PARTS[index].next;
            }
            return get_mode == 7 ? count : size;
        case 9:
        case 10:
            while (MEM_PARTS[index].state) {
                if (MEM_PARTS[index].state == get_arg) {
                    count++;
                    size += MEM_PARTS[index].size;
                }
                index = MEM_PARTS[index].next;
            }
            return get_mode == 9 ? count : size;
        case 11:
        case 12:
            while (MEM_PARTS[index].state) {
                if (MEM_PARTS[index].pid == get_arg && (MEM_PARTS[index].state == 4 || MEM_PARTS[index].state == 1)) {
                    count++;
                    size += MEM_PARTS[index].size;
                }
                index = MEM_PARTS[index].next;
            }
            return get_mode == 11 ? count : size;
        default: break;
    }

    return -1;
}
