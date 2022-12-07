// header file to use with tlib.c

#ifndef TLIB_ID
#define TLIB_ID 1234


#define get_func_addr ((int (*)(int, int)) *(int *) 0x199994)

#define tlib_sum ((int (*)(int, int)) get_func_addr(TLIB_ID, 2))
#define tlib_draw_rect ((void (*)(int, int, int, int, int)) get_func_addr(TLIB_ID, 3))
#define tlib_mhh ((int (*)(int, int)) get_func_addr(TLIB_ID, 4))

#endif
