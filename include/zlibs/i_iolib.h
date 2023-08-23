#ifndef IOLIB_ID
#define IOLIB_ID 1000

#include <type.h>

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

#define color_print ((uint32_t (*)(char *)) get_func_addr(IOLIB_ID, 3))
#define panda_color_print ((uint32_t (*)(char *, char, int)) get_func_addr(IOLIB_ID, 4))
#define rainbow_print ((void (*)(char *)) get_func_addr(IOLIB_ID, 5))
#define open_input ((void (*)(char *, int)) get_func_addr(IOLIB_ID, 6))

#endif
