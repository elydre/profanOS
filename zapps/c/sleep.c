/*****************************************************************************\
|   === sleep.c : 2024 ===                                                    |
|                                                                             |
|    Unix command implementation - sleep for a specified time      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// -1  error
//  0  sleep in seconds
//  1  sleep in milliseconds
//  2  get help
//  3  get unix time
//  4  get ms since boot

int parse_args(int argc, char **argv) {
    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0) {
            return 2;
        } else if (strcmp(argv[1], "-u") == 0) {
            return 3;
        } else if (strcmp(argv[1], "-b") == 0) {
            return 4;
        } else if (argv[1][0] == '-') {
            return -1;
        } else {
            return 0;
        }
    } else if (argc == 3) {
        if (strcmp(argv[1], "-m") == 0) {
            return 1;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

int main(int argc, char **argv) {
    int arg = parse_args(argc, argv);
    if (arg == -1) {
        fputs("sleep: Invalid arguments\nTry 'sleep -h' for more information.\n", stderr);
        return 1;
    } else if (arg == 2) {
        puts("Usage: sleep [args | time]\n"
            "args:\n"
            "  -b   print ms since boot\n"
            "  -h   print this help message\n"
            "  -u   print unix time\n"
            "  -m   sleep for <time> milliseconds\n"
        );
        return 0;
    } else if (arg == 3) {
        printf("%ld\n", time(NULL));
        return 0;
    } else if (arg == 4) {
        printf("%d\n", syscall_timer_get_ms());
        return 0;
    }

    int tosleep = atoi(argv[arg + 1]);
    if (tosleep <= 0) {
        fprintf(stderr, "sleep: '%s' is not a valid time\n", argv[arg + 1]);
        return 1;
    }

    if (arg == 0) {
        sleep(tosleep);
    } else if (arg == 1) {
        usleep(tosleep * 1000);
    }

    return 0;
}
