#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stddef.h>

void mem_copy(uint8_t *source, uint8_t *dest, int nbytes);
void mem_set(uint8_t *dest, uint8_t val, uint32_t len);

int mem_alloc(int size);
int mem_free_addr(int addr);

int mem_get_alloc_size(int addr);

void free(void *addr);
void * malloc(int size);
void * realloc(void * ptr, int size);
void * calloc(int size);

void mem_print();
int mem_get_usage();
int mem_get_usable();
int mem_get_alloc_count();
int mem_get_free_count();


#endif
