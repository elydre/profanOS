#ifndef MEM_H
#define MEM_H

#include <type.h>

void mem_copy(uint8_t *source, uint8_t *dest, int nbytes);
void mem_set(uint8_t *dest, uint8_t val, uint32_t len);
void mem_move(uint8_t *source, uint8_t *dest, int nbytes);

int mem_alloc(int size);
int mem_free_addr(int addr);

int mem_get_alloc_size(int addr);

void free(void *addr);
void *malloc(int size);
void *realloc(void *ptr, int size);
void *calloc(int size);

void mem_print();
int mem_get_usage();
int mem_get_usable();
int mem_get_alloc_count();
int mem_get_free_count();
int mem_get_phys_size();
int mem_get_base_addr();

#endif

