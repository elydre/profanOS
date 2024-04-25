/****** This file is part of profanOS **************************\
|   == clear.c ==                                    .pi0iq.    |
|                                                   d"  . `'b   |
|   Unix clear command implementation               q. /|\  u   |
|   clears the terminal screen                       `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <profan/syscall.h>
#include <stdio.h>

#define ANSI_CLEAR "\e[2J\e[H"

int ansiclear(void) {
    printf(ANSI_CLEAR);
    fflush(stdout);
    return 0;
}

int hardclear(void) {
    c_kprint(ANSI_CLEAR);
    ansiclear();
    return 0;
}

int showhelp(void) {
    puts(
        "Usage: clear [option]\n"
        " -a   use ANSI escape codes to clear the screen\n"
        " -h   show this help message\n"
        " -x   set all pixels to black and clear the screen"
    );
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) return ansiclear();
    if (argc > 2) return showhelp();
    if (argv[1][0] != '-') return showhelp();
    switch (argv[1][1]) {
        case 'a':
            return ansiclear();
        case 'h':
            return showhelp();
        case 'x':
            return hardclear();
        default:
            fprintf(stderr, "clear: invalid option -- '%c'\n", argv[1][1]);
            fputs("Try 'clear -h' for more information.\n", stderr);
            return 1;
    }
    return 0;
}
