/*****************************************************************************\
|   === speed.c : 2024 ===                                                    |
|                                                                             |
|    CPU speed test                                                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <string.h>
#include <stdio.h>

#define LOOPS 100000000

int check_args(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "0") == 0) {
        return 0;
    } else if (argc == 2 && strcmp(argv[1], "1") == 0) {
        return 1;
    }

    puts("Usage: speed [0|1]\n"
        "  0: keep scheduler enabled during the loop\n"
        "  1: disable scheduler during the loop\n\n"
        "This program will do a loop and print the time it took\n"
        "to do the loop. The loop is done 100 million times.\n"
        "The cpu speed is calculated by dividing the number of\n"
        "loops by the time it took to do the loop."
    );

    return 2;
}

int main(int argc, char **argv) {
    int mode = check_args(argc, argv);
    if (mode == 2)
        return 1;

    if (mode)
        syscall_process_auto_schedule(0);

    // get the time before the loop
    int start = syscall_timer_get_ms();

    // do a loop
    for (int i = 0; i < LOOPS; i++) {
        asm volatile("nop");
    }

    // get the time after the loop
    int end = syscall_timer_get_ms();

    if (mode)
        syscall_process_auto_schedule(1);

    // print the time difference
    printf("loop time: %dms\n", end - start);
    printf("cpu speed: %dMHz\n", ((LOOPS / 1000) * 8 / (end - start)));

    return 0;
}
