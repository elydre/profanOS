#ifndef WTRM_ID
#define WTRM_ID 1016

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

#define wterm_append_string ((void (*)(char *)) get_func_addr(WTRM_ID, 2))
#define wterm_append_char ((void (*)(char)) get_func_addr(WTRM_ID, 3))
#define wterm_get_buffer ((char *(*)(void)) get_func_addr(WTRM_ID, 4))
#define wterm_get_len ((int (*)(void)) get_func_addr(WTRM_ID, 5))

#endif
