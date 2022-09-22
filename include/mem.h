#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stddef.h>

void memory_copy(uint8_t *source, uint8_t *dest, int nbytes);
void memory_set(uint8_t *dest, uint8_t val, uint32_t len);

int alloc(int size);
int free_addr(int addr);


void free(void *addr);
void * malloc(int size);
void * realloc(void * ptr, int size);
void * calloc(int size);

void memory_print();
int get_memory_usage();
int get_usable_memory();


#endif
