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
#define PANDA_ID 6

#define PANDA_UNDERLINE 0x100

#include <stdint.h>

void panda_set_char(uint32_t x, uint32_t y, uint8_t c, uint16_t color);
uint16_t panda_print_string(const char *string, int len, int string_color, uint16_t default_color);
void panda_set_start(int kernel_cursor);
void panda_get_cursor(uint32_t *x, uint32_t *y);
void panda_draw_cursor(uint32_t x, uint32_t y);
void panda_get_size(uint32_t *cols, uint32_t *lines);
int  panda_change_font(uint32_t font_sid);

void *panda_screen_backup(void);
void  panda_screen_restore(void *backup);
void  panda_screen_free(void *backup);

#ifndef _KERNEL_MODULE

extern int profan_syscall(uint32_t id, ...);

#undef  _syscall
#define _syscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

#define panda_set_char(x, y, c, color)  ((void) _syscall(PANDA_ID, 0, x, y, c, color))
#define panda_print_string(str, len, str_color, def_color) \
        ((uint16_t) _syscall(PANDA_ID, 1, str, len, str_color, def_color))
#define panda_set_start(kernel_cursor)  ((void) _syscall(PANDA_ID, 2, kernel_cursor))
#define panda_get_cursor(x, y)          ((void) _syscall(PANDA_ID, 3, x, y))
#define panda_draw_cursor(x, y)         ((void) _syscall(PANDA_ID, 4, x, y))
#define panda_get_size(cols, lines)     ((void) _syscall(PANDA_ID, 5, cols, lines))
#define panda_change_font(font_sid)     ((int) _syscall(PANDA_ID, 6, font_sid))
#define panda_screen_backup()           ((void *) _syscall(PANDA_ID, 7, 0))
#define panda_screen_restore(backup)    ((void) _syscall(PANDA_ID, 8, backup))
#define panda_screen_free(backup)       ((void) _syscall(PANDA_ID, 9, backup))

#endif // _KERNEL_MODULE

#endif
