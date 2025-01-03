/*****************************************************************************\
|   === grep.c : 2024 ===                                                     |
|                                                                             |
|    Unix command implementation - search pattern in file          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#define GREP_COLOR 1
#define GREP_FNAME 2
#define GREP_ICASE 4

#define GREP_USAGE "Usage: grep [-i] <pattern> [file1] [file2] ...\n"

uint8_t g_flags;

char *grep_strstr(char *haystack, char *needle) {
    if (g_flags & GREP_ICASE) {
        for (char *h = haystack; *h; h++) {
            char *n = needle;
            for (char *h2 = h; *n && tolower(*n) == tolower(*h2); n++, h2++);
            if (!*n)
                return h;
        }
    } else {
        for (char *h = haystack; *h; h++) {
            char *n = needle;
            for (char *h2 = h; *n && *n == *h2; n++, h2++);
            if (!*n)
                return h;
        }
    }
    return NULL;
}

void print_with_color(char *line, char *pattern) {
    char *pattern_start, *pattern_end;

    if ((pattern_start = grep_strstr(line, pattern)) == NULL) {
        fputs(line, stdout);
        return;
    }

    fwrite(line, 1, pattern_start - line, stdout);
    pattern_end = pattern_start + strlen(pattern);

    fputs("\e[31m", stdout);
    fwrite(pattern_start, 1, pattern_end - pattern_start, stdout);
    fputs("\e[0m", stdout);

    print_with_color(pattern_end, pattern);
}

void grep_file(char *name, FILE *file, char *pattern, int flags) {
    char *line = NULL;
    size_t s;
    while (getline(&line, &s, file) != -1) {
        if (grep_strstr(line, pattern) == NULL)
            continue;
        if (g_flags & GREP_COLOR) {
            if (flags & GREP_FNAME)
                printf("\e[37m%s:\e[0m ", name);
            print_with_color(line, pattern);
        } else {
            if (flags & GREP_FNAME)
                printf("%s: ", name);
            fputs(line, stdout);
        }
    }
    free(line);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs(GREP_USAGE, stderr);
        return 1;
    }

    g_flags = isatty(1) ? GREP_COLOR : 0;

    if (strcmp(argv[1], "-i") == 0) {
        g_flags |= GREP_ICASE;
        argv++;
        argc--;
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            fputs(GREP_USAGE, stderr);
            return 1;
        }
    }

    char *pattern = argv[1];
    g_flags |= argc > 3 ? GREP_FNAME : 0;

    if (argc == 2) {
        grep_file("stdin", stdin, pattern, g_flags);
        return 0;
    }

    for (int i = 2; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            fprintf(stderr, "grep: %s: %s\n", argv[i], strerror(errno));
            return 1;
        }
        grep_file(argv[i], file, pattern, g_flags);
        fclose(file);
    }

    return 0;
}
