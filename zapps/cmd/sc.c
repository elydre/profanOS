/*****************************************************************************\
|   === sc.c : 2024 ===                                                       |
|                                                                             |
|    Test file                                                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define SYSCALL_CREATE_FUNCS

#include <profan/syscall.h>
#include <stdio.h>

int main(void) {

    printf("syscall vesa get info (0) -> %d\n", syscall_vesa_get_info(0));
    printf("syscall vesa get info (1) -> %d\n", syscall_vesa_get_info(1));

    printf("sizeof(sid_t) -> %d\n", sizeof(sid_t));

    return 0;
}
