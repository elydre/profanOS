#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stddef.h>

/* WARNING
the memory manager is not safe.
a update will be made to make it better.
*/

void memory_copy(uint8_t *source, uint8_t *dest, int nbytes);
void memory_set(uint8_t *dest, uint8_t val, uint32_t len);

/* At this stage there is no 'free' implemented. */
uint32_t kmalloc(size_t size, int align, uint32_t *phys_addr);
uint32_t alloc_page(int get_only);

#endif
