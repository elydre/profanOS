// header file to use with tlib.c

#ifndef TLIB_H
#define TLIB_H

#define WATDILY_ADDR 0x199994
#define LIB_ID 1234

#define get_func_addr ((int (*)(int, int)) *(int *) WATDILY_ADDR)

#define tlib_main ((int (*)(void)) get_func_addr(LIB_ID, 1))
#define tlib_draw_rect ((void (*)(int, int, int, int, int)) get_func_addr(LIB_ID, 2))
#define tlib_sum ((int (*)(int, int)) get_func_addr(LIB_ID, 3))

#endif
