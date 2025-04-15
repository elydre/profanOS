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
#include <profan/carp.h>

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
    carp_init("[-x]", 0);

    carp_register('x', CARP_STANDARD, "set all pixels to black and clear the screen");

    if (carp_parse(argc, argv))
        return 1;

    if (carp_isset('x'))
        hardclear();

    ansiclear();

    return 0;
}
