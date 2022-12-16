#ifndef MEM_H
#define MEM_H

#include <type.h>

void mem_copy(uint8_t *source, uint8_t *dest, int nbytes);
void mem_set(uint8_t *dest, uint8_t val, uint32_t len);
void mem_move(uint8_t *source, uint8_t *dest, int nbytes);

// SNOWFLAKE memory manager

typedef struct allocated_part_t {
    uint32_t addr;
    uint32_t size;
    int task_id;
    uint8_t state;
    int next; // list index
} allocated_part_t;

#define PARTS_COUNT 100

// states:
// 0 - free
// 1 - allocated
// 2 - initial block
// 3 - mm struct

void mem_init();
void mem_print();

uint32_t mem_get_alloc_size(uint32_t addr);

void free(void *addr);
void *malloc(uint32_t size);
void *realloc(void *ptr, uint32_t size);
void *calloc(uint32_t size);

int mem_get_usage();
int mem_get_usable();
int mem_get_alloc_count();
int mem_get_free_count();
int mem_get_phys_size();

#endif
