#ifndef WADDS_ID
#define WADDS_ID 1016

#include <i_libdaube.h>

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

#define wterm_append_string ((void (*)(char *)) get_func_addr(WADDS_ID, 2))
#define wterm_append_char ((void (*)(char)) get_func_addr(WADDS_ID, 3))
#define wterm_get_buffer ((char *(*)(void)) get_func_addr(WADDS_ID, 4))
#define wterm_get_len ((int (*)(void)) get_func_addr(WADDS_ID, 5))
#define wadds_create_exitbt ((button_t *(*)(window_t *, void (*)(clickevent_t *))) get_func_addr(WADDS_ID, 6))
#define wadds_line ((void (*)(window_t *, int, int, int, int, uint32_t)) get_func_addr(WADDS_ID, 7))
#define wadds_rect ((void (*)(window_t *, int, int, int, int, int)) get_func_addr(WADDS_ID, 8))
#define wadds_putc ((void (*)(window_t *, int, int, char, uint32_t, uint32_t)) get_func_addr(WADDS_ID, 9))
#define wadds_puts ((void (*)(window_t *, int, int, char*, uint32_t, uint32_t)) get_func_addr(WADDS_ID, 10))

#endif
