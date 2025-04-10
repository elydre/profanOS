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
#include <profan/cap.h>

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
    cap_init("[-x]", 0);

    cap_register('x', CAP_STANDARD, "set all pixels to black and clear the screen");

    if (cap_parse(argc, argv))
        return 1;

    if (cap_isset('x'))
        hardclear();

    ansiclear();

    return 0;
}
