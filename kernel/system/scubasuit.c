#include <kernel/scubasuit.h>
#include <kernel/snowflake.h>
#include <kernel/process.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>


scuba_directory_t *kernel_directory;
scuba_directory_t *current_directory;
uint32_t g_map_to_addr;

scuba_directory_t *scuba_get_kernel_directory(void) {
    return kernel_directory;
}

void *i_allign_calloc(uint32_t size, int state) {
    void *ptr = (void *) mem_alloc(size, 0x1000, state); // we need to allign to 4KB
    mem_set(ptr, 0, size);
    return ptr;
}


/**************************
 *                       *
 *      MMU CONTROL      *
 *                       *
**************************/

void scuba_enable(void) {
    // enable paging
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void scuba_switch(scuba_directory_t *dir) {
    current_directory = dir;
    // switch to the new page directory
    asm volatile("mov %0, %%cr3":: "r"(dir));
}

void scuba_flush_tlb(void) {
    // flush the TLB

    uint32_t cr3;
    asm volatile("mov %%cr3, %0": "=r"(cr3));
    asm volatile("mov %0, %%cr3":: "r"(cr3));
}

/**************************
 *                       *
 *      SCUBA INIT       *
 *                       *
**************************/

int scuba_init(void) {
    // allocate a page directory
    kernel_directory = scuba_directory_create(0);

    g_map_to_addr = SCUBA_MAP_TO;
    uint32_t physical_end = mem_get_info(0, 0);

    if (g_map_to_addr > physical_end) {
        g_map_to_addr = physical_end;
        sys_warning("Cannot map memory to desired limit");
    }

    // map the memory to itself
    for (uint32_t i = 0; i < g_map_to_addr; i += 0x1000) {
        scuba_map(kernel_directory, i, i);
    }

    // video memory
    if (vesa_does_enable()) {
        uint32_t from = (uint32_t) vesa_get_framebuffer();
        uint32_t to = from + vesa_get_pitch() * vesa_get_height() * 4 + 0x1000;
        for (uint32_t i = from; i < to; i += 0x1000) {
            scuba_map(kernel_directory, i, i);
        }
    }

    // switch to the new page directory
    scuba_switch(kernel_directory);

    // enable paging
    scuba_enable();

    return 0;
}

/**************************
 *                       *
 *     SCUBA PROCESS     *
 *                       *
**************************/

void scuba_process_switch(scuba_directory_t *dir) {
    if (current_directory == dir) return;
    if (dir->pid == 1) return;

    // switch to the new page directory
    scuba_switch(dir);

    // flush the TLB
    scuba_flush_tlb();
}

/**************************
 *                       *
 *    SCUBA DIRECTORY    *
 *                       *
**************************/

scuba_directory_t *scuba_directory_create(int target_pid) {
    // allocate a page directory
    scuba_directory_t *dir = i_allign_calloc(sizeof(scuba_directory_t), 6);

    dir->to_free_index = 0;
    dir->pid = target_pid;

    // setup directory entries
    for (int i = 0; i < 1024; i++) {
        dir->entries[i].present = 0;
        dir->entries[i].rw = 1;
        dir->entries[i].user = 1;
        dir->entries[i].accessed = 0;
        dir->entries[i].unused = 0;
        dir->entries[i].frame = 0;
    }

    return dir;
}

void scuba_directory_init(scuba_directory_t *dir) {
    // map the memory to itself
    for (uint32_t i = 0; i < g_map_to_addr; i += 0x1000) {
        scuba_map_from_kernel(dir, i, i);
    }

    // video memory
    uint32_t from, to;

    if (vesa_does_enable()) {
        // pixel buffer
        from = (uint32_t) vesa_get_framebuffer();
        to = from + vesa_get_pitch() * vesa_get_height() * 4 + 0x1000;
    }

    for (uint32_t i = from; i < to; i += 0x1000) {
        scuba_map_from_kernel(dir, i, i);
    }
}

void scuba_directory_destroy(scuba_directory_t *dir) {
    // free all page tables
    for (uint32_t i = 0; i < dir->to_free_index; i++) {
        free(dir->to_free[i]);
    }

    // free the page directory
    free(dir);
}

/**************************
 *                       *
 *   SCUBA MAP / UNMAP   *
 *                       *
**************************/

int scuba_map_func(scuba_directory_t *dir, uint32_t virt, uint32_t phys, int from_kernel) {
    // get the page table index
    uint32_t table_index = virt / 0x1000 / 1024;

    // get the page table
    scuba_page_table_t *table = dir->tables[table_index];

    // if the page table doesn't exist, create it
    if (!table) {
        if (from_kernel) {
            // use the kernel page table
            table = kernel_directory->tables[table_index];
            if (!table) {
                sys_error("Cannot use non-existant kernel page table");
                return 1;
            }
            if (!table->pages[(virt / 0x1000) % 1024].present) {
                sys_error("Address not mapped in kernel page table");
                return 1;
            }
        } else {
            // check if we have space to free
            if (dir->to_free_index >= SCUBA_MAX_TO_FREE) {
                sys_error("Too many page tables to free");
                return 1;
            }

            // create a new page table
            table = i_allign_calloc(sizeof(scuba_page_table_t), 6);
            dir->to_free[dir->to_free_index++] = table;
        }
        dir->tables[table_index] = table;

        dir->entries[table_index].present = 1;
        dir->entries[table_index].rw = 1;
        dir->entries[table_index].user = 1;
        dir->entries[table_index].accessed = 0;
        dir->entries[table_index].unused = 0;
        dir->entries[table_index].frame = (uint32_t) table / 0x1000;
    }

    if (from_kernel) return 0;

    // get the page index
    uint32_t page_index = (virt / 0x1000) % 1024;

    // map the page
    table->pages[page_index].frame = phys / 0x1000;
    table->pages[page_index].present = 1;
    table->pages[page_index].rw = 1;
    table->pages[page_index].user = 1;

    return 0;
}

int scuba_create_virtual(scuba_directory_t *dir, uint32_t virt, int count) {
    if (dir->to_free_index + count >= SCUBA_MAX_TO_FREE) {
        sys_error("Too many pages to free");
        return 1;
    }

    // alloc a page
    uint32_t phys = (uint32_t) i_allign_calloc(0x1000 * count, 7);

    if (!phys) {
        sys_error("Failed to alloc page");
        return 1;
    }

    // add the page to the list of pages to free
    dir->to_free[dir->to_free_index++] = (void *) phys;

    // map the page
    for (int i = 0; i < count; i++) {
        if (!scuba_map(dir, virt + i * 0x1000, phys + i * 0x1000)) continue;
        sys_error("Failed to map page");
        return 1;
    }

    return 0;
}

int scuba_unmap(scuba_directory_t *dir, uint32_t virt) {
    // get the page table index
    uint32_t table_index = virt / 0x1000 / 1024;

    // get the page table
    scuba_page_table_t *table = dir->tables[table_index];

    // if the page table doesn't exist, return
    if (!table) {
        sys_error("Page table doesn't exist");
        return 1;
    }

    // get the page index
    uint32_t page_index = (virt / 0x1000) % 1024;

    // unmap the page
    table->pages[page_index].present = 0;
    return 0;
}

/**************************
 *                       *
 *    SCUBA GET PHYS     *
 *     ( not used )      *
**************************/

uint32_t scuba_get_phys(scuba_directory_t *dir, uint32_t virt) {
    // get the page table index
    uint32_t table_index = virt / 0x1000 / 1024;

    // get the page table
    scuba_page_table_t *table = dir->tables[table_index];

    // if the page table doesn't exist, return
    if (!table) return 0;

    // get the page index
    uint32_t page_index = (virt / 0x1000) % 1024;

    // get the page
    scuba_page_t *page = &table->pages[page_index];

    // if the page doesn't exist, return
    if (!page->present) return 0;

    // return the physical address
    return page->frame * 0x1000;
}

/***************************
 *                        *
 *    SCUBA PAGE FAULT    *
 *                        *
***************************/

void scuba_fault_handler(int err_code) {
    // get the faulting address
    uint32_t faulting_address, new_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int pid = process_get_pid();

    // check if the faulting address is after RUN_BIN_VBASE
    if (faulting_address >= RUN_BIN_VBASE) {
        new_address = faulting_address - (faulting_address % 0x1000);
        if (scuba_create_virtual(current_directory, new_address, RUN_BIN_VEXPD)) {
            sys_error("Failed to create virtual pages");
        } else {
            serial_kprintf("Created virtual pages for %x\n", faulting_address);
            return;
        }
    } else {
        kprintf("Page fault during %s at %x, pid %d, code %x\n",
                (err_code & 0x2) ? "write" : "read",
                faulting_address,
                pid,
                err_code
        );
        sys_error("Page fault, killing process");
    }

    // exit with the standard segfault code
    if (force_exit_pid(pid, 139)) {
        sys_fatal("Failed to exit process");
    }
}
