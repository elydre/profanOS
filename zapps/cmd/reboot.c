/*****************************************************************************\
|   === reboot.c : 2024 ===                                                   |
|                                                                             |
|    Command to reboot or shutdown the system                      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>

int main(int argc) {
    if (argc == 1)
        syscall_sys_reboot();
    syscall_sys_shutdown();
    return 1;
}
