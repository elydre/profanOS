#include <kernel/scubasuit.h>
#include <kernel/snowflake.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>


scuba_directory_t *kernel_directory;
scuba_directory_t *current_directory;

scuba_directory_t *scuba_get_kernel_directory() {
    return kernel_directory;
}

void *i_allign_calloc(size_t size, int state) {
    void *ptr = (void *) mem_alloc(size, 0x1000, state); // we need to allign to 4KB
    mem_set(ptr, 0, size);
    return ptr;
}

int scuba_init() {
    // allocate a page directory
    kernel_directory = scuba_directory_create(0);

    // map the first 16MB of memory
    for (int i = 0; i < 0x4000000; i += 0x1000) {
        scuba_map(kernel_directory, i, i);
    }

    // video memory
    if (vesa_does_enable()) {
        uint32_t from = (uint32_t) vesa_get_framebuffer();
        uint32_t to = from + vesa_get_width() * vesa_get_height() * 4 + 0x1000;
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

void scuba_enable() {
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

void scuba_flush_tlb() {
    // flush the TLB

    uint32_t cr3;
    asm volatile("mov %%cr3, %0": "=r"(cr3));
    asm volatile("mov %0, %%cr3":: "r"(cr3));
}

void scuba_get_current_directory() {
    // dont use current_directory because it might be NULL
    uint32_t cr3;
    asm volatile("mov %%cr3, %0": "=r"(cr3));
}

/**************************
 *                       *
 *     SCUBA PROCESS     *
 *                       *
**************************/

#include <kernel/process.h>

void scuba_process_switch(scuba_directory_t *dir) {
    if (current_directory == dir) return;
    if (dir->pid == 1) return;

    serial_kprintf("switching to process %d\n", dir->pid);

    // switch to the new page directory
    scuba_switch(dir);

    // flush the TLB
    scuba_flush_tlb();
    scuba_get_current_directory();
}

/**************************
 *                       *
 *    SCUBA DIRECTORY    *
 *                       *
**************************/

scuba_directory_t *scuba_directory_create(int target_pid) {
    // allocate a page directory
    scuba_directory_t *dir = i_allign_calloc(sizeof(scuba_directory_t), 6);
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
    // kernel, lib, alloc, from 1Mo to 16Mo
    for (int i = 0; i < 0x4000000; i += 0x1000) {
        scuba_map(dir, i, i);
    }

    // video memory
    uint32_t from, to;

    if (vesa_does_enable()) {
        // pixel buffer
        from = (uint32_t) vesa_get_framebuffer();
        to = from + vesa_get_width() * vesa_get_height() * 4 + 0x1000;
    } else {
        // text mode
        from = 0xB8000;
        to = from + 80 * 25 * 2 + 0x1000;
    }

    for (uint32_t i = from; i < to; i += 0x1000) {
        scuba_map(dir, i, i);
    }
}

void scuba_directory_destroy(scuba_directory_t *dir) {
    serial_kprintf("destroying directory %d\n", dir->pid);

    // free all page tables
    for (int i = 0; i < 1024; i++) {
        if (dir->tables[i]) {
            serial_kprintf("destroying table %d\n", i);
            free(dir->tables[i]);
        }
    }

    // free the page directory
    free(dir);
}

/**************************
 *                       *
 *   SCUBA MAP / UNMAP   *
 *                       *
**************************/

void scuba_map(scuba_directory_t *dir, uint32_t virt, uint32_t phys) {
    // get the page table index
    uint32_t table_index = virt / 0x1000 / 1024;

    // get the page table
    scuba_page_table_t *table = dir->tables[table_index];

    // if the page table doesn't exist, create it
    if (!table) {
        serial_kprintf("creating table %d\n", table_index);

        table = i_allign_calloc(sizeof(scuba_page_table_t), 6);
        dir->tables[table_index] = table;

        dir->entries[table_index].present = 1;
        dir->entries[table_index].rw = 1;
        dir->entries[table_index].user = 1;
        dir->entries[table_index].accessed = 0;
        dir->entries[table_index].unused = 0;
        dir->entries[table_index].frame = (uint32_t) table / 0x1000;
    }

    // get the page index
    uint32_t page_index = (virt / 0x1000) % 1024;

    // map the page
    table->pages[page_index].frame = phys / 0x1000;
    table->pages[page_index].present = 1;
    table->pages[page_index].rw = 1;
    table->pages[page_index].user = 1;
}

void scuba_unmap(scuba_directory_t *dir, uint32_t virt) {
    // get the page table index
    uint32_t table_index = virt / 0x1000 / 1024;

    // get the page table
    scuba_page_table_t *table = dir->tables[table_index];

    // if the page table doesn't exist, return
    if (!table) return;

    // get the page index
    uint32_t page_index = (virt / 0x1000) % 1024;

    // unmap the page
    table->pages[page_index].present = 0;
}

/**************************
 *                       *
 *    SCUBA GET PHYS     *
 *                       *
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
