/*****************************************************************************\
|   === syscall.h : 2024 ===                                                  |
|                                                                             |
|    Syscall handler header                                        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <cpu/isr.h>

void syscall_handler(registers_t *r);
