/****** This file is part of profanOS **************************\
|   == mouse.h ==                                    .pi0iq.    |
|                                                   d"  . `'b   |
|   Mouse aliases for userland                      q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef I_MOUSE_H
#define I_MOUSE_H

#include <profan/syscall.h>

#define mouse_get_x() c_mouse_call(0, 0)
#define mouse_get_y() c_mouse_call(1, 0)

#define mouse_get_button(button) c_mouse_call(2, button)

#define mouse_set_x(x) c_mouse_call(3, x)
#define mouse_set_y(y) c_mouse_call(4, y)

#define mouse_reset() c_mouse_call(5, 0)

#endif
