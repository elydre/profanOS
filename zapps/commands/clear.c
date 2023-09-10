#include <syscall.h>
#include <stdio.h>

int ansiclear() {
    printf("\033[2J");
    return 0;
}

int hardclear() {
    c_kprint("\033[2J");
    ansiclear();
    return 0;
}

int showhelp() {
    puts(
        "Usage: clear [OPTION]\n"
        " -a   use ANSI escape codes to clear the screen\n"
        " -h   show this help message\n"
        " -x   set all pixels to black and clear the screen\n"
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
            return showhelp();
    }
    return 0;
}
