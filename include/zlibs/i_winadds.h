#ifndef WADDS_ID
#define WADDS_ID 1016

#include <i_libdaube.h>

#define WADDS_NOBG 0xFF000000

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

#define wadds_line ((void (*)(window_t *, int, int, int, int, uint32_t, uint8_t)) get_func_addr(WADDS_ID, 2))
#define wadds_rect ((void (*)(window_t *, int, int, int, int, int, uint8_t)) get_func_addr(WADDS_ID, 3))
#define wadds_putc ((void (*)(window_t *, char, int, int, uint32_t, uint32_t, uint8_t)) get_func_addr(WADDS_ID, 4))
#define wadds_puts ((void (*)(window_t *, char*, int, int, uint32_t, uint32_t, uint8_t)) get_func_addr(WADDS_ID, 5))
#define wadds_fill ((void (*)(window_t *, uint32_t, uint8_t)) get_func_addr(WADDS_ID, 6))
#define wadds_get_kb ((int (*)(window_t *)) get_func_addr(WADDS_ID, 7))
#define wadds_draw_bmp ((int (*)(window_t *, char*, int, int)) get_func_addr(WADDS_ID, 8))

#endif
