#ifndef IOLIB_ID
#define IOLIB_ID 1000

#include <type.h>

#define HEX_BLUE     0x5555FF
#define HEX_GREEN    0x55FF55
#define HEX_CYAN     0x55FFFF
#define HEX_RED      0xFF5555
#define HEX_MAGENTA  0xFF55FF
#define HEX_YELLOW   0xFFFF55
#define HEX_GREY     0xAAAAAA
#define HEX_WHITE    0xFFFFFF
#define HEX_DBLUE    0x0000AA
#define HEX_DGREEN   0x00AA00
#define HEX_DCYAN    0x00AAAA
#define HEX_DRED     0xAA0000
#define HEX_DMAGENTA 0xAA00AA
#define HEX_DYELLOW  0xAAAA00
#define HEX_DGREY    0x404040
#define HEX_DWHITE   0xC0C0C0


#ifndef IOLIB_C

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
void color_print(char *s);
void rainbow_print(char *message);
void input_wh(char *out_buffer, int size, char color, char ** history, int history_size);
*/


#define input(out_buffer, size, color) input_wh(out_buffer, size, color, NULL, 0)

#define color_print ((uint32_t (*)(char *)) get_func_addr(IOLIB_ID, 3))
#define rainbow_print ((void (*)(char *)) get_func_addr(IOLIB_ID, 4))
#define input_wh ((void (*)(char *, int, uint32_t, char **, int)) get_func_addr(IOLIB_ID, 5))

#endif
#endif
