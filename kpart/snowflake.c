#include <kernel/snowflake.h>
#include <kernel/process.h>
#include <driver/serial.h>
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
int first_part_index;
uint32_t alloc_count;
uint32_t free_count;
uint32_t phys_size;
int part_size;

uint32_t mem_alloc(uint32_t size, int state);
int mem_free_addr(uint32_t addr);

int mem_get_phys_size() {
    int *addr_min = (int *) 0x200000;
    int *addr_max = (int *) 0x40000000;
    int *addr_test;
    while (addr_max - addr_min > 1) {
        addr_test = addr_min + (addr_max - addr_min) / 2;
        *addr_test = 0x1234;
        if (*addr_test == 0x1234) addr_min = addr_test;
        else addr_max = addr_test;
    }
    return (int) addr_max;
}

int mem_init() {
    first_part_index = 0;
    alloc_count = 0;
    free_count = 0;

    phys_size = mem_get_phys_size();

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

    return 0;
}

int mm_get_unused_index() {
    for (int i = 0; i < part_size; i++) {
        if (MEM_PARTS[i].state == 0) return i;
    }

    sys_error("no more memory, dynamizing do not work");
    return -1;
}

void del_occurence(int index) {
    int i = first_part_index;
    while (MEM_PARTS[i].state) {
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
        if (sum > 4) return;
    }

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
    if (!(size && state)) return 0;
    if (state != 3) dynamize_mem();

    // traversing the list of allocated parts
    int index, old_index, exit_mode;
    uint32_t last_addr;
    index = first_part_index;

    last_addr = MEM_BASE_ADDR;
    while (1) {
        // if the part is free
        if (MEM_PARTS[index].state == 0) {
            exit_mode = 1 + (last_addr + size > phys_size);
            break;
        }
        if (MEM_PARTS[index].addr - last_addr >= size) {
            // we can allocate the part here
            exit_mode = 0;
            break;
        }
        last_addr = MEM_PARTS[index].addr + MEM_PARTS[index].size;

        old_index = index;
        index = MEM_PARTS[index].next;
    }

    if (exit_mode == 2) {
        sys_error("Cannot alloc: out of physical memory");
        return 0;
    }

    int new_index = mm_get_unused_index();
    if (new_index == -1) return 0;

    int i = exit_mode ? index: new_index;

    MEM_PARTS[i].addr = last_addr;
    MEM_PARTS[i].size = size;
    MEM_PARTS[i].task_id = (state == 1) ? process_get_running_pid(): 0;
    MEM_PARTS[i].state = state;

    if (exit_mode) {
        new_index = mm_get_unused_index();
        if (new_index == -1) return 0;
        MEM_PARTS[index].next = new_index;
    } else {
        del_occurence(new_index);

        MEM_PARTS[old_index].next = new_index;
        MEM_PARTS[new_index].next = index;
    }
    alloc_count++;
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
                free_count++;
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

void mem_free_all(int task_id) {
    uint32_t index = first_part_index;
    while (MEM_PARTS[index].state) {
        if (MEM_PARTS[index].task_id == task_id) {
            mem_free_addr(MEM_PARTS[index].addr);
        }
        index = MEM_PARTS[index].next;
    }
}

int mem_get_info(char get_mode, int get_arg) {
    // check the header for informations about the get_mode

    if (get_mode == 0) return phys_size;
    if (get_mode == 1) return MEM_BASE_ADDR;
    if (get_mode == 2) return sizeof(allocated_part_t) * part_size;
    if (get_mode == 3) return (int) MEM_PARTS;
    if (get_mode == 4) return alloc_count;
    if (get_mode == 5) return free_count;

    int index = first_part_index;

    int info[7];
    for (int i = 0; i < 7; i++) info[i] = 0;

    while (MEM_PARTS[index].state) {
        info[0] += MEM_PARTS[index].size;
        if (MEM_PARTS[index].task_id == get_arg && MEM_PARTS[index].state > 0) {
            info[1]++;
            info[2] += MEM_PARTS[index].size;
        }
        if (MEM_PARTS[index].state == 4) { // bin_run
            info[3]++;
            info[4] += MEM_PARTS[index].size;
        }
        if (MEM_PARTS[index].state == 5) { // libs
            info[5]++;
            info[6] += MEM_PARTS[index].size;
        }
        index = MEM_PARTS[index].next;
    }

    if (get_mode > 5 && get_mode < 13) return info[get_mode - 6];
    return -1;
}
