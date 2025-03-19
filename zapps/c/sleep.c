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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char **argv) {
    if (argc != 2 || argv[1][0] == '-') {
        fprintf(stderr, "Usage: %s <seconds>\n", argv[0]);
        return 1;
    }

    for (int i = 0; argv[1][i]; i++) {
        if ((argv[1][i] < '0' || argv[1][i] > '9') && argv[1][i] != '.') {
            fprintf(stderr, "%s: invalid time interval\n", argv[0]);
            return 1;
        }
    }

    double seconds = atof(argv[1]);

    if (seconds < 0) {
        fprintf(stderr, "%s: invalid time interval\n", argv[0]);
        return 1;
    }

    struct timespec req = {
        .tv_sec = (time_t) seconds,
        .tv_nsec = (long) ((seconds - (time_t) seconds) * 1e9)
    };

    if (nanosleep(&req, NULL) == -1) {
        perror(argv[0]);
        return 1;
    }

    return 0;
}
