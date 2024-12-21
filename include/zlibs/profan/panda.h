/*****************************************************************************\
|   === panda.h : 2024 ===                                                    |
|                                                                             |
|    Kernel module header for panda terminal emulator              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef PANDA_ID
#define PANDA_ID 1006

#include <stdint.h>

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define panda_set_char ((void (*)(uint32_t, uint32_t, uint8_t, uint8_t)) get_func_addr(PANDA_ID, 2))
#define panda_print_string ((uint8_t (*)(const char *, int, int, uint8_t)) get_func_addr(PANDA_ID, 3))
#define panda_set_start ((void (*)(int)) get_func_addr(PANDA_ID, 4))
#define panda_get_cursor ((void (*)(uint32_t *, uint32_t *)) get_func_addr(PANDA_ID, 5))
#define panda_draw_cursor ((void (*)(uint32_t, uint32_t)) get_func_addr(PANDA_ID, 6))
#define panda_get_size ((void (*)(uint32_t *, uint32_t *)) get_func_addr(PANDA_ID, 7))
#define panda_change_font ((int (*)(uint32_t)) get_func_addr(PANDA_ID, 8))
#define panda_screen_backup ((void *(*)(void)) get_func_addr(PANDA_ID, 9))
#define panda_screen_restore ((void (*)(void *)) get_func_addr(PANDA_ID, 10))
#define panda_screen_free ((void (*)(void *)) get_func_addr(PANDA_ID, 11))

#endif
