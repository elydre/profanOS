/*****************************************************************************\
|   === scubasuit.c : 2024 ===                                                |
|                                                                             |
|    Kernel virtual memory manager                                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/scubasuit.h>
#include <kernel/snowflake.h>
#include <kernel/process.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>


scuba_directory_t *kernel_directory;
uint32_t g_map_to_addr;

/**************************
 *                       *
 *     INTERNAL UTIL     *
 *                       *
**************************/

void *i_allign_calloc(uint32_t size) {
    void *ptr = (void *) mem_alloc(size, 0x1000, 7); // we need to allign to 4KB
    mem_set(ptr, 0, size);
    return ptr;
}

scuba_directory_t *i_directory_create(void) {
    // allocate a page directory
    scuba_directory_t *dir = i_allign_calloc(sizeof(scuba_directory_t));

    dir->to_free_index = 0;

    // setup directory entries
    for (int i = 0; i < 1024; i++) {
        dir->entries[i].rw = 1;
        dir->entries[i].user = 1;
    }

    return dir;
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
 *    SCUBA DIRECTORY    *
 *                       *
**************************/

scuba_directory_t *scuba_get_kernel_directory(void) {
    return kernel_directory;
}

scuba_directory_t *scuba_directory_inited(void) {
    scuba_directory_t *dir = i_directory_create();

    // map the memory to itself
    for (uint32_t i = 0x1000; i < g_map_to_addr; i += 0x1000) {
        scuba_map_from_kernel(dir, (void *) i, (void *) i);
    }

    // video memory
    uint32_t from, to;

    if (!vesa_does_enable()) {
        return dir;
    }

    // pixel buffer
    from = (uint32_t) vesa_get_fb();
    to = from + vesa_get_pitch() * vesa_get_height() * 4 + 0x1000;

    for (uint32_t i = from; i < to; i += 0x1000) {
        scuba_map_from_kernel(dir, (void *) i, (void *) i);
    }

    return dir;
}

scuba_directory_t *scuba_directory_copy(scuba_directory_t *dir) {
    // allocate a new page directory
    scuba_directory_t *new_dir = i_directory_create();

    // copy the page tables
    for (int i = 0; i < 1024; i++) {
        if (!dir->entries[i].frame) continue;

        // get the page table
        scuba_page_table_t *table = (scuba_page_table_t *) (dir->entries[i].frame * 0x1000);

        // if the page is from the kernel use the kernel page table
        if (dir->entries[i].fkernel) {
            new_dir->entries[i] = dir->entries[i];
            continue;
        }

        // allocate a new page table
        scuba_page_table_t *new_table = i_allign_calloc(sizeof(scuba_page_table_t));

        // copy the page table
        for (int j = 0; j < 1024; j++) {
            new_table->pages[j] = table->pages[j];
            if (!(table->pages[j].present && table->pages[j].deepcopy))
                continue;
            // clone the page
            uint32_t data = mem_alloc(0x1000, 0x1000, 7);
            mem_copy((void *) data, (void *) (table->pages[j].frame * 0x1000), 0x1000);
            new_table->pages[j].frame = (uint32_t) data / 0x1000;
            new_table->pages[j].allocate = 1;
        }

        // map the new page table
        new_dir->entries[i] = dir->entries[i];
        new_dir->entries[i].frame = (uint32_t) new_table / 0x1000;
        new_dir->entries[i].present = 1;
    }

    return new_dir;
}

void scuba_directory_destroy(scuba_directory_t *dir) {
    // free all pages (if not from kernel)
    for (uint32_t i = 0; i < 1024; i++) {
        if (!dir->entries[i].frame || dir->entries[i].fkernel)
            continue;

        // get the page table
        scuba_page_table_t *table = (scuba_page_table_t *) (dir->entries[i].frame * 0x1000);

        // free all pages
        for (uint32_t j = 0; j < 1024; j++) {
            if (table->pages[j].present && table->pages[j].allocate) {
                free((void *) (table->pages[j].frame * 0x1000));
            }
        }
        free(table);
    }

    for (uint32_t i = 0; i < dir->to_free_index; i++) {
        free(dir->to_free[i]);
    }

    // free the page directory
    free(dir);
}

/**************************
 *                       *
 *      SCUBA INIT       *
 *                       *
**************************/

int scuba_init(void) {
    // allocate a page directory
    kernel_directory = i_directory_create();

    g_map_to_addr = mem_get_info(0, 0);

    // map the memory to itself
    for (uint32_t i = 0x1000; i < g_map_to_addr; i += 0x1000) {
        scuba_map_func(kernel_directory, (void *) i, (void *) i, 2);
    }

    // video memory
    if (vesa_does_enable()) {
        uint32_t from = (uint32_t) vesa_get_fb();
        uint32_t to = from + vesa_get_pitch() * vesa_get_height() * 4 + 0x1000;
        for (uint32_t i = from; i < to; i += 0x1000) {
            scuba_map_func(kernel_directory, (void *) i, (void *) i, 2);
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
 *   SCUBA MAP / UNMAP   *
 *                       *
**************************/

// mode 0: normal
// mode 1: use kernel table
// mode 2: is kernel
// mode 3: child no copy
int scuba_map_func(scuba_directory_t *dir, void *virt, void *phys, int mode) {
    // get the page table index
    uint32_t table_index = (uint32_t) virt / 0x1000 / 1024;
    int fkernel = (mode == 1 || mode == 2);

    // get the page table
    scuba_page_table_t *table = (void *) (dir->entries[table_index].frame * 0x1000);
    // if the page table doesn't exist, create it
    if (!table) {
        if (mode == 1) {
            // use the kernel page table
            table = (void *) (kernel_directory->entries[table_index].frame * 0x1000);
            if (!table) {
                sys_error("Cannot use non-existant kernel page (pid: %d, addr: %x)", process_get_pid(), virt);
                return 1;
            }
            if (!table->pages[((uint32_t) virt / 0x1000) % 1024].present) {
                sys_error("Address not mapped in kernel page (pid: %d, addr: %x)", process_get_pid(), virt);
                return 1;
            }
        } else {
            // create a new page table
            table = i_allign_calloc(sizeof(scuba_page_table_t));
        }
        dir->entries[table_index].present = 1;
        dir->entries[table_index].rw = 1;
        dir->entries[table_index].user = 1;
        dir->entries[table_index].accessed = 0;
        dir->entries[table_index].dirty = 0;
        dir->entries[table_index].fkernel = fkernel;
        dir->entries[table_index].frame = (uint32_t) table / 0x1000;
    } else if (dir->entries[table_index].fkernel && !fkernel) {
        sys_error("Cannot map to kernel page (pid: %d, addr: %x)", process_get_pid(), virt);
        return 1;
    }

    if (mode == 1) return 0;

    // get the page index
    uint32_t page_index = ((uint32_t) virt / 0x1000) % 1024;

    // map the page
    table->pages[page_index].frame = (uint32_t) phys / 0x1000;
    table->pages[page_index].present = 1;
    table->pages[page_index].rw = 1;
    table->pages[page_index].user = 1;
    table->pages[page_index].deepcopy = (mode == 0);

    return 0;
}

void *scuba_create_virtual(scuba_directory_t *dir, void *virt, uint32_t count) {
    if (dir->to_free_index + 1 >= SCUBA_MAX_TO_FREE) {
        sys_error("Too many pages to free (pid: %d)", process_get_pid());
        return 0;
    }

    // check if the pages are already mapped
    for (uint32_t i = 0; i < count; i++) {
        if (scuba_get_phys(dir, virt + i * 0x1000)) {
            sys_error("Address already mapped (pid: %d, addr: %x)", process_get_pid(), virt + i * 0x1000);
            return 0;
        }
    }

    // alloc a page
    void *phys = i_allign_calloc(0x1000 * count);

    if (!phys) {
        sys_error("Failed to alloc page (pid: %d)", process_get_pid());
        return 0;
    }

    // add the page to the list of pages to free
    dir->to_free[dir->to_free_index++] = phys;

    // map the page
    for (uint32_t i = 0; i < count; i++) {
        if (!scuba_map(dir, virt + i * 0x1000, phys + i * 0x1000)) continue;
        sys_error("Failed to map page (pid: %d, addr: %x)", process_get_pid(), virt + i * 0x1000);
        return 0;
    }

    return phys;
}

int scuba_unmap(scuba_directory_t *dir, void *virt) {
    // get the page table index
    uint32_t table_index = (uint32_t) virt / 0x1000 / 1024;

    // get the page table
    scuba_page_table_t *table = (void *) (dir->entries[table_index].frame * 0x1000);

    // if the page table doesn't exist, return
    if (!table) {
        sys_error("Page table doesn't exist (pid: %d, addr: %x)", process_get_pid(), virt);
        return 1;
    }

    // get the page index
    uint32_t page_index = ((uint32_t) virt / 0x1000) % 1024;

    // unmap the page
    table->pages[page_index].present = 0;
    return 0;
}

/**************************
 *                       *
 *    SCUBA GET PHYS     *
 *                       *
**************************/

void *scuba_get_phys(scuba_directory_t *dir, void *virt) {
    // get the page table index
    uint32_t table_index = (uint32_t) virt / 0x1000 / 1024;

    // get the page table
    scuba_page_table_t *table = (void *) (dir->entries[table_index].frame * 0x1000);

    // if the page table doesn't exist, return
    if (!table) return 0;

    // get the page index
    uint32_t page_index = ((uint32_t) virt / 0x1000) % 1024;

    // get the page
    scuba_page_t *page = &table->pages[page_index];

    // if the page doesn't exist, return
    if (!page->present) return 0;

    // return the physical address
    return (void *) (page->frame * 0x1000);
}

/*************************
 *                      *
 *    SCUBA SYSCALLS    *
 *                      *
*************************/

void *scuba_call_generate(void *addr, uint32_t size) {
    return scuba_create_virtual(process_get_directory(process_get_pid()), addr, size);
}

int scuba_call_map(void *addr, void *phys, int cic) {
    return scuba_map_func(process_get_directory(process_get_pid()), addr, phys, cic ? 0 : 3);
}

int scuba_call_unmap(void *addr) {
    return scuba_unmap(process_get_directory(process_get_pid()), addr);
}

void *scuba_call_phys(void *addr) {
    return (void *) scuba_get_phys(process_get_directory(process_get_pid()), addr);
}

/***************************
 *                        *
 *    SCUBA PAGE FAULT    *
 *                        *
***************************/

void scuba_fault_handler(int err_code) {
    // get the faulting address
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int pid = process_get_pid();

    // check if the faulting address is after RUN_BIN_VBASE
    sys_error("Page fault during %s at %x (pid %d, code %x)",
            (err_code & 0x2) ? "write" : "read",
            faulting_address,
            pid,
            err_code
    );
}
