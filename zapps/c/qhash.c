/*****************************************************************************\
|   === qhash.c : 2025 ===                                                    |
|                                                                             |
|    Quick hash tool for file integrity check                      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

uint64_t make_qhash(FILE *file) {
    unsigned char buf[512];
    uint64_t qhash = 0;
    size_t bytes_read;

    while ((bytes_read = fread(buf, 1, sizeof(buf), file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++)
            qhash = (qhash << 1) ^ (buf[i] ^ (qhash >> 31));
    }

    return ferror(file) ? 0 : qhash;
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-' && argv[i][1] == '\0')
                fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[i]);
            else
                fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], argv[i][1]);
            return 1;
        }

        FILE *file = fopen(argv[i], "rb");

        if (file == NULL) {
            fprintf(stderr, "%s: %s: %m\n", argv[0], argv[i]);
            continue;
        }

        printf("%016" PRIx64 "  %s\n", make_qhash(file), argv[i]);

        fclose(file);
    }

    return 0;
}
