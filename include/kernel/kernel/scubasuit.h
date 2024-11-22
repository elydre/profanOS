/*****************************************************************************\
|   === scubasuit.h : 2024 ===                                                |
|                                                                             |
|    Kernel virtual memory manager header                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef SCUBASUIT_H
#define SCUBASUIT_H

#include <cpu/isr.h>
#include <ktype.h>

// SCUBASUIT virtual memory manager

// stadard page size
#define PAGE_SIZE 4096
#define SCUBA_MAX_TO_FREE 512

// Max 4KB per page
typedef struct {
    uint32_t present :  1;
    uint32_t rw :       1;
    uint32_t user :     1;
    uint32_t accessed : 1;
    uint32_t dirty :    1;
    uint32_t unused :   5;
    uint32_t deepcopy : 1;
    uint32_t allocate : 1;
    uint32_t frame :   20;
} scuba_page_t;

// Max 4MB per table
typedef struct {
    scuba_page_t pages[1024];
} scuba_page_table_t;

typedef struct {
    uint32_t present :  1;
    uint32_t rw :       1;
    uint32_t user :     1;
    uint32_t accessed : 1;
    uint32_t dirty :    1;
    uint32_t unused :   6;
    uint32_t fkernel :  1;
    uint32_t frame :   20; // page table
} scuba_dir_entry_t;

// Max 4GB per directory
typedef struct {
    scuba_dir_entry_t entries[1024];
    uint32_t pid;
} scuba_dir_t;

#define scuba_map(dir, virt, phys) scuba_map_func(dir, virt, phys, 0)
#define scuba_map_from_kernel(dir, virt, phys) scuba_map_func(dir, virt, phys, 1)

scuba_dir_t *scuba_get_kernel_dir(void);

// init, switch
int scuba_init(void);

void scuba_switch(scuba_dir_t *dir);
#define scuba_switch(dir) asm volatile("mov %0, %%cr3":: "r"(dir))

// directory creation, destruction
scuba_dir_t *scuba_dir_inited(scuba_dir_t *parent, uint32_t pid);
scuba_dir_t *scuba_dir_copy(scuba_dir_t *dir, uint32_t pid);

void scuba_dir_destroy(scuba_dir_t *dir);

void scuba_dump(scuba_dir_t *dir);

// map, unmap
int scuba_map_func(scuba_dir_t *dir, void *virt, void *phys, int mode);
int scuba_unmap(scuba_dir_t *dir, void *virt);
void *scuba_create_virtual(scuba_dir_t *dir, void *virt, uint32_t count);

// get physical, fault handler
void *scuba_get_phys(scuba_dir_t *dir, void *virt);
void scuba_fault_handler(int err_code);

// syscall functions
void *scuba_call_generate(void *addr, uint32_t size);
int scuba_call_map(void *addr, void *phys, int cic);
int scuba_call_unmap(void *addr);
void *scuba_call_phys(void *addr);

#endif
