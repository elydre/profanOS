#ifndef PROFAN_LIB_ID
#define PROFAN_LIB_ID 1002

#include <type.h>

#define KB_LEFT 75
#define KB_RIGHT 77
#define KB_OLDER 72
#define KB_NEWER 80
#define KB_CTRL 29
#define KB_ESC 1
#define KB_A 16
#define KB_Q 30
#define KB_E 18
#define KB_R 19
#define KB_D 32
#define KB_Z 17
#define KB_S 31
#define KB_released_value 128

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define assemble_path ((void (*)(char *, char *, char *)) get_func_addr(PROFAN_LIB_ID, 2))
#define profan_print_stacktrace ((void (*)(void)) get_func_addr(PROFAN_LIB_ID, 3))
#define profan_print_memory ((void (*)(void *, uint32_t)) get_func_addr(PROFAN_LIB_ID, 4))
#define profan_kb_load_map ((int (*)(char *)) get_func_addr(PROFAN_LIB_ID, 5))
#define profan_kb_get_char ((char (*)(uint8_t, uint8_t)) get_func_addr(PROFAN_LIB_ID, 6))

#endif
