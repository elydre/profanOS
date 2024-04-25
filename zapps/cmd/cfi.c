/****** This file is part of profanOS **************************\
|   == cfi.c ==                                      .pi0iq.    |
|                                                   d"  . `'b   |
|   Checksum File Integrity                         q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

unsigned buffer_hash(char *buffer, int length) {
    unsigned hash = 0;

    for (int i = 0; i < length; i++) {
        hash += buffer[i] * (i + 1);
    }

    return hash;
}

unsigned build_number(char *file) {
    unsigned number = 0;
    FILE *fp;
    char line[1024];

    fp = fopen(file, "r");
    if (fp == NULL) {
        fprintf(stderr, "cfi: %s: File not found\n", file);
        exit(1);
    }

    unsigned count;

    while ((count = fread(line, 1, sizeof(line), fp)) > 0) {
        number += buffer_hash(line, count);
    }

    fclose(fp);
    return number;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [file2] [...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            fprintf(stderr, "cfi: %s: Invalid option\n", argv[i]);
            return 1;
        }
        printf("%s: %x\n", argv[i], build_number(argv[i]));
    }

    return 0;
}
