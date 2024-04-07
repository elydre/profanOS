#ifndef PANDA_ID
#define PANDA_ID 1006

#include <profan/type.h>

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define panda_set_char ((void (*)(uint32_t, uint32_t, char, char)) get_func_addr(PANDA_ID, 7))
#define panda_print_string ((void (*)(char *, int, int)) get_func_addr(PANDA_ID, 12))
#define panda_set_start ((void (*)(int)) get_func_addr(PANDA_ID, 13))
#define panda_get_cursor ((void (*)(uint32_t *, uint32_t *)) get_func_addr(PANDA_ID, 14))
#define panda_draw_cursor ((void (*)(uint32_t, uint32_t)) get_func_addr(PANDA_ID, 15))
#define panda_get_size ((void (*)(uint32_t *, uint32_t *)) get_func_addr(PANDA_ID, 16))
#define panda_change_font ((int (*)(char *)) get_func_addr(PANDA_ID, 17))

#endif
