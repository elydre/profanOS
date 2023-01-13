#ifndef MEM_ID
#define MEM_ID 1003

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
void mem_copy(uint8_t *source, uint8_t *dest, int nbytes);
void mem_set(uint8_t *dest, uint8_t val, uint32_t len);
void mem_move(uint8_t *source, uint8_t *dest, int nbytes);

void free(void *addr);
void *malloc(uint32_t size);
void *realloc(void *ptr, uint32_t size);
void *calloc(uint32_t size);
*/

#define mem_copy ((void (*)(uint8_t *, uint8_t *, int)) get_func_addr(MEM_ID, 2))
#define mem_set ((void (*)(uint8_t *, uint8_t, uint32_t)) get_func_addr(MEM_ID, 3))
#define mem_move ((void (*)(uint8_t *, uint8_t *, int)) get_func_addr(MEM_ID, 4))

#define free ((void (*)(void *)) get_func_addr(MEM_ID, 5))
#define malloc ((void *(*)(uint32_t)) get_func_addr(MEM_ID, 6))
#define realloc ((void *(*)(void *, uint32_t)) get_func_addr(MEM_ID, 7))
#define calloc ((void *(*)(uint32_t)) get_func_addr(MEM_ID, 8))

#endif
