#include <kernel/scubasuit.h>
#include <minilib.h>

page_directory_t *kernel_directory;


page_directory_t *scuba_get_kernel_directory() {
    return kernel_directory;
}



void scuba_switch(page_directory_t *dir) {
    // set the page directory
    asm volatile("mov %0, %%cr3":: "r"(dir->physical_addr));

    // enable paging
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

int scuba_init() {
    // allocate a page directory
    kernel_directory = calloc(sizeof(page_directory_t));

    // map the first 16MB of memory
    for (int i = 0; i < 0x4000000; i += 0x1000) {
        scuba_map(kernel_directory, i, i + 0xC0000000);
    }

    return 0;
}

/**************************
 *                       *
 *   SCUBA MAP / UNMAP   *
 *                       *
**************************/

void scuba_map(page_directory_t *dir, uint32_t virt, uint32_t phys) {
    // get the page table index
    uint32_t table_index = virt / 0x1000 / 1024;

    // get the page table
    page_table_t *table = dir->tables[table_index];

    // if the page table doesn't exist, create it
    if (!table) {
        serial_kprintf("creating table %d\n", table_index);
        table = calloc(sizeof(page_table_t));
        dir->tables[table_index] = table;
        dir->tables_physical[table_index] = (uint32_t) table | 0x7;
    }

    // get the page index
    uint32_t page_index = (virt / 0x1000) % 1024;

    // map the page
    table->pages[page_index].frame = phys / 0x1000;
    table->pages[page_index].present = 1;
    table->pages[page_index].rw = 1;
    table->pages[page_index].user = 1;
}

void scuba_unmap(page_directory_t *dir, uint32_t virt) {
    // get the page table index
    uint32_t table_index = virt / 0x1000 / 1024;

    // get the page table
    page_table_t *table = dir->tables[table_index];

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

uint32_t scuba_get_phys(page_directory_t *dir, uint32_t virt) {
    // get the page table index
    uint32_t table_index = virt / 0x1000 / 1024;
    kprintf("table index: %x\n", table_index);

    // get the page table
    page_table_t *table = dir->tables[table_index];
    kprintf("table: %x\n", table);

    // if the page table doesn't exist, return
    if (!table) return 0;

    // get the page index
    uint32_t page_index = (virt / 0x1000) % 1024;

    kprintf("page index: %x\n", page_index);

    // get the page
    page_t *page = &table->pages[page_index];
    kprintf("page: %x\n", page);

    // if the page doesn't exist, return
    if (!page->present) return 0;
    kprintf("page frame: %x\n", page->frame);

    // return the physical address
    return page->frame * 0x1000;
}
