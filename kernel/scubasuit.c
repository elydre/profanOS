#include <kernel/scubasuit.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>


scuba_directory_t *kernel_directory;

scuba_directory_t *scuba_get_kernel_directory() {
    return kernel_directory;
}

// TODO: use memory manager to allign this
void *i_allign_calloc(size_t size) {
    void *ptr = calloc(size + 0x1000);
    ptr = (void *) (((uint32_t) ptr + 0x1000) & 0xFFFFF000);
    return ptr;
}

int scuba_init() {
    // allocate a page directory
    kernel_directory = i_allign_calloc(sizeof(scuba_directory_t));

    // setup directory entries
    for (int i = 0; i < 1024; i++) {
        kernel_directory->entries[i].present = 0;
        kernel_directory->entries[i].rw = 1;
        kernel_directory->entries[i].user = 1;
        kernel_directory->entries[i].accessed = 0;
        kernel_directory->entries[i].unused = 0;
        kernel_directory->entries[i].frame = 0;
    }

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
    // switch to the new page directory
    asm volatile("mov %0, %%cr3":: "r"(dir));
}

void scuba_flush_tlb() {
    // flush the TLB
    asm volatile("mov %%cr3, %%eax"::);
    asm volatile("mov %%eax, %%cr3"::);
}

/**************************
 *                       *
 *   PROCESS SWITCHING   *
 *                       *
**************************/



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

        table = i_allign_calloc(sizeof(scuba_page_table_t));
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
    kprintf("getting phys for %x\n", virt);
    // get the page table index
    uint32_t table_index = virt / 0x1000 / 1024;
    kprintf("table index: %x\n", table_index);

    // get the page table
    scuba_page_table_t *table = dir->tables[table_index];
    kprintf("table: %x\n", table);

    // if the page table doesn't exist, return
    if (!table) return 0;

    // get the page index
    uint32_t page_index = (virt / 0x1000) % 1024;

    kprintf("page index: %x\n", page_index);

    // get the page
    scuba_page_t *page = &table->pages[page_index];
    kprintf("page: %x\n", page);

    // if the page doesn't exist, return
    if (!page->present) return 0;
    kprintf("page frame: %x\n", page->frame);

    // return the physical address
    return page->frame * 0x1000;
}
