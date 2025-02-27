/*****************************************************************************\
|   === mouse.h : 2024 ===                                                    |
|                                                                             |
|    Mouse aliases for userland                                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_MOUSE_H
#define _PROFAN_MOUSE_H

#include <profan/syscall.h>

static int mod_mouse_call(int a1, int a2) {
    int a;
    asm volatile(
        "int $0x80"
        : "=a" (a)
        : "a" (0x09000002), "b" (a1), "c" (a2)
    ); \
    return a; \
}

#define mouse_get_x() mod_mouse_call(0, 0)
#define mouse_get_y() mod_mouse_call(1, 0)

#define mouse_get_button(button) mod_mouse_call(2, button)

#define mouse_set_x(x) mod_mouse_call(3, x)
#define mouse_set_y(y) mod_mouse_call(4, y)

#define mouse_reset() mod_mouse_call(5, 0)

#endif
