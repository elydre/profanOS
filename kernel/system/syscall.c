/*****************************************************************************\
|   === syscall.c : 2024 ===                                                  |
|                                                                             |
|    Syscall handler                                               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/syscall.h>
#include <minilib.h>
#include <system.h>

void syscall_handler(registers_t *r) {
    kprintf("syscall %d\n", r->eax);
}
