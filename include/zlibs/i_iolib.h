#ifndef IOLIB_ID
#define IOLIB_ID 1000

#include <type.h>

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
void color_print(char *s);
void rainbow_print(char message[]);
void input_wh(char out_buffer[], int size, char color, char ** history, int history_size);
*/

#define input(out_buffer, size, color) input_wh(out_buffer, size, color, NULL, 0)

#define color_print ((uint32_t (*)(char *)) get_func_addr(IOLIB_ID, 3))
#define rainbow_print ((void (*)(char *)) get_func_addr(IOLIB_ID, 4))
#define input_wh ((void (*)(char *, int, char, char **, int)) get_func_addr(IOLIB_ID, 5))


#endif
