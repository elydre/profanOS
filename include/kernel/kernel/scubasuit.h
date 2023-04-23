#ifndef SCUBASUIT_H
#define SCUBASUIT_H

#include <cpu/isr.h>
#include <type.h>

// Max 4KB per page
typedef struct {
    uint32_t present :  1;
    uint32_t rw :       1;
    uint32_t user :     1;
    uint32_t accessed : 1;
    uint32_t dirty :    1;
    uint32_t unused :   7;
    uint32_t frame :   20;
} page_t;

// Max 4MB per table
typedef struct {
    page_t pages[1024]; 
} page_table_t;

typedef struct {
    uint32_t present :  1;
    uint32_t rw :       1;
    uint32_t user :     1;
    uint32_t accessed : 1;
    uint32_t unused :   4;
    uint32_t frame :   20;
} page_dir_entry_t;

// Max 4GB per directory
typedef struct {
    page_dir_entry_t entries[1024];
    page_table_t *tables[1024];
} page_directory_t;

// SCUBASUIT virtual memory manager

#define PAGE_SIZE 4096


int scuba_init();

void scuba_map(page_directory_t *dir, uint32_t virt, uint32_t phys);
void scuba_unmap(page_directory_t *dir, uint32_t virt);

void scuba_switch(page_directory_t *dir);
uint32_t scuba_get_phys(page_directory_t *dir, uint32_t virt);

page_directory_t *scuba_get_kernel_directory();


void scuba_fault_handler(registers_t *reg);

#endif
