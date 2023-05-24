#ifndef OCMLIB_ID
#define OCMLIB_ID 1005

#include <i_libdaube.h>
#include <type.h>

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
void ocm_init(window_t *window)
void ocm_print(char *message, int x, int y, uint32_t color, uint32_t bg_color)
int ocm_get_cursor_offset()
void ocm_set_cursor_offset(int offset)
void ocm_cursor_blink(int on)
void ocm_clear()
int ocm_get_max_rows()
int ocm_get_max_cols()
*/

#define ocm_init ((void (*)(window_t *)) get_func_addr(OCMLIB_ID, 2))
#define ocm_print ((void (*)(char *, int, int, uint32_t, uint32_t)) get_func_addr(OCMLIB_ID, 6))
#define ocm_get_cursor_offset ((int (*)(void)) get_func_addr(OCMLIB_ID, 7))
#define ocm_set_cursor_offset ((void (*)(int)) get_func_addr(OCMLIB_ID, 8))
#define ocm_cursor_blink ((void (*)(int)) get_func_addr(OCMLIB_ID, 9))
#define ocm_clear ((void (*)(void)) get_func_addr(OCMLIB_ID, 10))
#define ocm_get_max_rows ((int (*)(void)) get_func_addr(OCMLIB_ID, 11))
#define ocm_get_max_cols ((int (*)(void)) get_func_addr(OCMLIB_ID, 12))

#endif
