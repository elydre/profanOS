#ifndef IOLIB_ID
#define IOLIB_ID 1000

#include <type.h>

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define rainbow_print ((void (*)(char *)) get_func_addr(IOLIB_ID, 3))
#define open_input ((uint32_t (*)(char *, uint32_t)) get_func_addr(IOLIB_ID, 4))

#endif
