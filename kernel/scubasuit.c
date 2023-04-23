#include <kernel/scubasuit.h>
#include <minilib.h>
#include <system.h>


page_directory_t *kernel_directory;

page_directory_t *scuba_get_kernel_directory() {
    return kernel_directory;
}


int scuba_init() {
    // allocate a page directory
    kernel_directory = calloc(sizeof(page_directory_t) + 0x1000);

    // align the page directory
    kernel_directory = (page_directory_t *) (((uint32_t) kernel_directory + 0x1000) & 0xFFFFF000);

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
        scuba_map(kernel_directory, i + 0xC0000000, i);
    }

    // setup the page fault handler
    register_interrupt_handler(IRQ14, scuba_fault_handler);

    // switch to the new page directory
    scuba_switch(kernel_directory);
    while (1);

    return 0;
}

void scuba_switch(page_directory_t *dir) {
    // set the page directory
    asm volatile("mov %0, %%cr3":: "r"(dir));

    // enable paging
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "r"(cr0));    
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


/**************************
 *                       *
 *   SCUBA PAGE FAULT    *
 *                       *
**************************/

// Err code interpretation
#define ERR_PRESENT     0x1
#define ERR_RW          0x2
#define ERR_USER        0x4
#define ERR_RESERVED    0x8
#define ERR_INST        0x10

void scuba_fault_handler(registers_t *reg) {
    asm volatile("sti");
    serial_kprintf("Page fault:\n");

    // Gather fault info and print to screen
    uint32_t faulting_addr;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_addr));

    uint8_t present = reg->err_code & ERR_PRESENT;
    uint8_t rw = reg->err_code & ERR_RW;
    uint8_t user = reg->err_code & ERR_USER;
    uint8_t reserved = reg->err_code & ERR_RESERVED;
    uint8_t inst_fetch = reg->err_code & ERR_INST;

    serial_kprintf("Faulting address: %x\n", faulting_addr);

    serial_kprintf("Possible causes: [ ");
    if(!present) serial_kprintf("Page not present ");
    if(rw) serial_kprintf("Page is read only ");
    if(user) serial_kprintf("Page is read only ");
    if(reserved) serial_kprintf("Overwrote reserved bits ");
    if(inst_fetch) serial_kprintf("Instruction fetch ");
    serial_kprintf("]\n");

    sys_fatal("Page fault");
}
