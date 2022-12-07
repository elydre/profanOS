// header file to use with tlib.c

#ifndef TLIB_ID
#define TLIB_ID 1234

#define WATDILY_ADDR 0x199994

#define get_func_addr(func_id) ((int (*)(int, int)) *(int *) WATDILY_ADDR)(TLIB_ID, func_id)

#define tlib_sum ((int (*)(int, int)) get_func_addr(2))
#define tlib_draw_rect ((void (*)(int, int, int, int, int)) get_func_addr(3))
#define tlib_mhh ((int (*)(int, int)) get_func_addr(4))

#endif
