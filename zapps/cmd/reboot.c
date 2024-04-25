/****** This file is part of profanOS **************************\
|   == reboot.c ==                                   .pi0iq.    |
|                                                   d"  . `'b   |
|   Unix reboot command implementation              q. /|\  u   |
|   reboots and shuts down the system                `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <profan/syscall.h>

int main(int argc, char **argv) {
    if (argc == 1)
        c_sys_reboot();
    c_sys_shutdown();
    return 1;
}
