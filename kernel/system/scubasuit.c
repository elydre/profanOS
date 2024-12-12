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

scuba_dir_t *kernel_directory;

/**************************
 *                       *
 *     INTERNAL UTIL     *
 *                       *
**************************/

static void *i_allign_calloc(uint32_t size, scuba_dir_t *dir) {
    void *ptr = mem_alloc_dir(size, 0x1000, dir->pid, dir);
    mem_set(ptr, 0, size);
    return ptr;
}

static scuba_dir_t *i_directory_create(uint32_t pid) {
    // allocate a page directory
    scuba_dir_t *dir = mem_alloc_dir(sizeof(scuba_dir_t), 0x1000, pid, NULL);
    mem_set(dir, 0, sizeof(scuba_dir_t));
    dir->pid = pid;

    // setup directory entries
    for (int i = 0; i < 1024; i++) {
        dir->entries[i].rw = 1;
        dir->entries[i].user = 1;
    }

    return dir;
}

/**************************
 *                       *
 *    SCUBA DIRECTORY    *
 *                       *
**************************/

scuba_dir_t *scuba_get_kernel_dir(void) {
    return kernel_directory;
}

scuba_dir_t *scuba_dir_inited(scuba_dir_t *parent, uint32_t pid) {
    scuba_dir_t *dir = i_directory_create(pid);

    // copy all page from parent noted as kernel
    for (int i = 0; i < 1024; i++) {
        if (!parent->entries[i].frame || !parent->entries[i].fkernel) continue;
        dir->entries[i] = parent->entries[i];
    }

    // deep copy the bin exec memory
    void *addr = scuba_create_virtual(dir, (void *) RUN_HEAP_ADDR, RUN_HEAP_SIZE / 0x1000);
    void *phys = scuba_get_phys(parent, (void *) RUN_HEAP_ADDR);
    if (phys) {
        mem_copy(addr, phys, RUN_HEAP_SIZE);
    } else {
        mem_set(addr, 0, RUN_HEAP_SIZE);
    }

    return dir;
}

scuba_dir_t *scuba_dir_copy(scuba_dir_t *dir, uint32_t pid) {
    // allocate a new page directory
    scuba_dir_t *new_dir = i_directory_create(pid);

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
        scuba_page_table_t *new_table = i_allign_calloc(sizeof(scuba_page_table_t), new_dir);

        // copy the page table
        for (int j = 0; j < 1024; j++) {
            new_table->pages[j] = table->pages[j];
            if (!(table->pages[j].present && table->pages[j].deepcopy))
                continue;
            // clone the page
            void *data = mem_alloc_dir(0x1000, 0x1000, new_dir->pid, new_dir);
            mem_copy(data, (void *) (table->pages[j].frame * 0x1000), 0x1000);
            new_table->pages[j].frame = (uint32_t) data / 0x1000;
        }

        // map the new page table
        new_dir->entries[i] = dir->entries[i];
        new_dir->entries[i].frame = (uint32_t) new_table / 0x1000;
        new_dir->entries[i].present = 1;
    }

    return new_dir;
}

void scuba_dir_destroy(scuba_dir_t *dir) {
    mem_free_dir(dir);
    free(dir);
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
int scuba_map_func(scuba_dir_t *dir, void *virt, void *phys, int mode) {
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
            table = i_allign_calloc(sizeof(scuba_page_table_t), dir);
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

void *scuba_create_virtual(scuba_dir_t *dir, void *virt, uint32_t count) {
    // check if the pages are already mapped
    for (uint32_t i = 0; i < count; i++) {
        if (scuba_get_phys(dir, virt + i * 0x1000)) {
            sys_error("Address already mapped (pid: %d, addr: %x)", process_get_pid(), virt + i * 0x1000);
            return 0;
        }
    }

    // alloc a page
    void *phys = i_allign_calloc(0x1000 * count, dir);

    // map the page
    for (uint32_t i = 0; i < count; i++) {
        if (!scuba_map(dir, virt + i * 0x1000, phys + i * 0x1000)) continue;
        sys_error("Failed to map page (pid: %d, addr: %x)", process_get_pid(), virt + i * 0x1000);
        return 0;
    }

    return phys;
}

int scuba_unmap(scuba_dir_t *dir, void *virt) {
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
 *      SCUBA INIT       *
 *                       *
**************************/

int scuba_init(void) {
    uint32_t cr0, map_to_addr;

    kernel_directory = i_directory_create(0);
    map_to_addr = mem_get_info(0, 0);

    // map the memory to itself
    for (uint32_t i = 0x1000; i < map_to_addr; i += 0x1000) {
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
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "r"(cr0));

    return 0;
}

/**************************
 *                       *
 *    SCUBA GET PHYS     *
 *                       *
**************************/

void *scuba_get_phys(scuba_dir_t *dir, void *virt) {
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

/**************************
 *                       *
 *      SCUBA DUMP       *
 *                       *
**************************/

void scuba_dump(scuba_dir_t *dir) {
    kprintf_serial("Dumping page directory %x\n", dir);

    for (int i = 0; i < 1024; i++) {
        if (!dir->entries[i].frame) continue;

        scuba_page_table_t *table = (void *) (dir->entries[i].frame * 0x1000);

        if (dir->entries[i].fkernel) {
            kprintf_serial("  page table %d [%x to %x] %x (from kernel)\n", i,
                    i * 1024 * 0x1000, (i + 1) * 1024 * 0x1000, table);
            continue;
        }

        kprintf_serial("  page table %d [%x to %x] %x\n", i, i * 1024 * 0x1000, (i + 1) * 1024 * 0x1000, table);

        for (int j = 0; j < 1024; j++) {
            if (!table->pages[j].frame) continue;

            kprintf_serial("    page %d: %x -> %x\n", j, (i * 1024 + j) * 0x1000, table->pages[j].frame * 0x1000);
        }
    }
}

/*************************
 *                      *
 *    SCUBA SYSCALLS    *
 *                      *
*************************/

void *scuba_call_generate(void *addr, uint32_t size) {
    return scuba_create_virtual(process_get_dir(process_get_pid()), addr, size);
}

int scuba_call_map(void *addr, void *phys, int cic) {
    return scuba_map_func(process_get_dir(process_get_pid()), addr, phys, cic ? 0 : 3);
}

int scuba_call_unmap(void *addr) {
    return scuba_unmap(process_get_dir(process_get_pid()), addr);
}

void *scuba_call_phys(void *addr) {
    return (void *) scuba_get_phys(process_get_dir(process_get_pid()), addr);
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

    if (pid == 0)
        sys_fatal("Page fault in the kernel during %s at %x)",
                (err_code & 0x2) ? "write" : "read",
                faulting_address
        );

    sys_error("Page fault during %s at %x (pid %d, code %x)",
            (err_code & 0x2) ? "write" : "read",
            faulting_address,
            pid,
            err_code
    );
}
