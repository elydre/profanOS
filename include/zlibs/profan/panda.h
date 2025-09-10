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
#define PANDA_ID 5

#define PANDA_UNDERLINE 0x100

#include <stdint.h>

typedef struct {
    uint16_t color;
    char   content;
} panda_char_t;

void     panda_print_char(uint32_t x, uint32_t y, uint8_t c, uint16_t color); // slow on large texts
void     panda_print_raw(panda_char_t *buffer, uint32_t length);
uint16_t panda_print_string(const char *string, int len, int string_color, uint16_t default_color);

void panda_set_start(int kernel_cursor);
void panda_get_size(uint32_t *cols, uint32_t *lines);
void panda_get_cursor(uint32_t *x, uint32_t *y);
void panda_draw_cursor(uint32_t x, uint32_t y);

int panda_change_font(uint32_t font_sid);

void *panda_screen_backup(void);
void  panda_screen_restore(void *backup);
void  panda_screen_free(void *backup);

#ifndef _KERNEL_MODULE

extern int profan_syscall(uint32_t id, ...);

#undef  _pscall
#define _pscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

#define panda_print_char(x, y, c, color)  ((void) _pscall(PANDA_ID, 0, x, y, c, color))
#define panda_print_raw(buffer, length)   ((void) _pscall(PANDA_ID, 1, buffer, length))
#define panda_print_string(str, len, str_color, def_color) \
        ((uint16_t) _pscall(PANDA_ID, 2, str, len, str_color, def_color))

#define panda_set_start(kernel_cursor)  ((void) _pscall(PANDA_ID, 3, kernel_cursor))
#define panda_get_size(cols, lines)     ((void) _pscall(PANDA_ID, 4, cols, lines))
#define panda_get_cursor(x, y)          ((void) _pscall(PANDA_ID, 5, x, y))
#define panda_draw_cursor(x, y)         ((void) _pscall(PANDA_ID, 6, x, y))

#define panda_change_font(font_sid)     ((int) _pscall(PANDA_ID, 7, font_sid))

#define panda_screen_backup()           ((void *) _pscall(PANDA_ID, 8, 0))
#define panda_screen_restore(backup)    ((void) _pscall(PANDA_ID, 9, backup))
#define panda_screen_free(backup)       ((void) _pscall(PANDA_ID, 10, backup))

#endif // _KERNEL_MODULE

#endif
