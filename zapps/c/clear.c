/*****************************************************************************\
|   === clear.c : 2024 ===                                                    |
|                                                                             |
|    Unix command implementation - clears the terminal screen      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/syscall.h>
#include <profan/arp.h>

#include <stdio.h>

#define ANSI_CLEAR "\e[2J\e[H"

void ansiclear(void) {
    fputs(ANSI_CLEAR, stdout);
    fflush(stdout);
}

void hardclear(void) {
    syscall_kprint(ANSI_CLEAR);
}

int main(int argc, char **argv) {
    arp_init("[-x]", 0);

    arp_register('x', ARP_STANDARD, "set all pixels to black and clear the screen");

    if (arp_parse(argc, argv))
        return 1;

    if (arp_isset('x'))
        hardclear();

    ansiclear();

    return 0;
}
