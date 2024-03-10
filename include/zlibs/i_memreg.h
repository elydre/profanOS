#ifndef MEMREG_ID
#define MEMREG_ID 1004

#include <type.h>

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define memreg_alloc ((void (*)(uint32_t, uint32_t, int, const char *)) get_func_addr(MEMREG_ID, 3))
#define memreg_free ((void (*)(uint32_t, int, const char *)) get_func_addr(MEMREG_ID, 6))
#define memreg_dump ((void (*)(int, int)) get_func_addr(MEMREG_ID, 7))
#define memreg_clean ((void (*)(int)) get_func_addr(MEMREG_ID, 8))

#endif
