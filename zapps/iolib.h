// header file to use with tlib.c

#ifndef IOLIB_ID
#define IOLIB_ID 1000

#define get_func_addr ((int (*)(int, int)) *(int *) 0x199994)

/*
void mskprint(int nb_args, ...);
void fskprint(char format[], ...);
void rainbow_print(char message[]);
void input_wh(char out_buffer[], int size, char color, char ** history, int history_size);
void input(char out_buffer[], int size, char color);
*/

#define mskprint ((void (*)(int, ...)) get_func_addr(IOLIB_ID, 4))
#define fskprint ((void (*)(char[], ...)) get_func_addr(IOLIB_ID, 5))
#define rainbow_print ((void (*)(char[])) get_func_addr(IOLIB_ID, 6))
#define input_wh ((void (*)(char[], int, char, char **, int)) get_func_addr(IOLIB_ID, 7))
#define input ((void (*)(char[], int, char)) get_func_addr(IOLIB_ID, 8))

#endif
