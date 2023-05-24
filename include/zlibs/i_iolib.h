#ifndef IOLIB_ID
#define IOLIB_ID 1000

#include <type.h>

#define HEX_BLUE     0x0000FF
#define HEX_GREEN    0x00FF00
#define HEX_CYAN     0x00FFFF
#define HEX_RED      0xFF0000
#define HEX_MAGENTA  0xFF00FF
#define HEX_YELLOW   0xFFFF00
#define HEX_GREY     0x808080
#define HEX_WHITE    0xFFFFFF
#define HEX_DBLUE    0x000080
#define HEX_DGREEN   0x008000
#define HEX_DCYAN    0x008080
#define HEX_DRED     0x800000
#define HEX_DMAGENTA 0x800080
#define HEX_DYELLOW  0x808000
#define HEX_DGREY    0x404040
#define HEX_DWHITE   0xC0C0C0


#ifndef IOLIB_C

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
void color_print(char *s);
void rainbow_print(char message[]);
void input_wh(char out_buffer[], int size, char color, char ** history, int history_size);
*/


#define input(out_buffer, size, color) input_wh(out_buffer, size, color, NULL, 0)

#define color_print ((uint32_t (*)(char *)) get_func_addr(IOLIB_ID, 3))
#define rainbow_print ((void (*)(char *)) get_func_addr(IOLIB_ID, 4))
#define input_wh ((void (*)(char *, int, uint32_t, char **, int)) get_func_addr(IOLIB_ID, 5))

#endif
#endif
