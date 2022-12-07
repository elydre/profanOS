// header file to use with tlib.c

#ifndef IOLIB_ID
#define IOLIB_ID 1000

#define WATDILY_ADDR 0x199994

#define get_func_addr(func_id) ((int (*)(int, int)) *(int *) WATDILY_ADDR)(IOLIB_ID, func_id)

/*
void mskprint(int nb_args, ...);
void fskprint(char format[], ...);
void rainbow_print(char message[]);
void input_wh(char out_buffer[], int size, char color, char ** history, int history_size);
void input(char out_buffer[], int size, char color);
*/

#define mskprint ((void (*)(int, ...)) get_func_addr(4))
#define fskprint ((void (*)(char[], ...)) get_func_addr(5))
#define rainbow_print ((void (*)(char[])) get_func_addr(6))
#define input_wh ((void (*)(char[], int, char, char **, int)) get_func_addr(7))
#define input ((void (*)(char[], int, char)) get_func_addr(8))

#endif
