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
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

#define GREP_COLOR 1
#define GREP_FNAME 2
#define GREP_LINES 4
#define GREP_ICASE 8
#define GREP_ALLLN 16

#define GREP_USAGE "Usage: grep [options] <pattern> [file1] [file2] ...\n"

uint32_t g_flags;
int      g_context;

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

void print_line(char *name, int lnum, char *line, char *pattern) {
    if (g_flags & GREP_COLOR) {
        if (g_flags & GREP_FNAME)
            printf("\e[37m%s:\e[0m ", name);
        if (g_flags & GREP_LINES)
            printf("\e[32m%d\e[90m:\e[0m ", lnum);
        print_with_color(line, pattern);
    } else {
        if (g_flags & GREP_FNAME)
            printf("%s: ", name);
        if (g_flags & GREP_LINES)
            printf("%d: ", lnum);
        fputs(line, stdout);
    }
}

void grep_file_ctx(char *name, FILE *file, char *pattern) {
    char *line = NULL;
    size_t s;

    char **context = malloc(g_context * sizeof(char *));
    int context_count = 0;
    int first = 1;

    for (int lnum = 0; getline(&line, &s, file) != -1; lnum++) {
        if (grep_strstr(line, pattern) != NULL) {
            if (first) first = 0;
            else fputs("--\n", stdout);

            for (int i = 0; i < context_count; i++) {
                print_line(name, lnum - context_count + i, context[i], pattern);
                free(context[i]);
            }

            print_line(name, lnum, line, pattern);

            for (int i = 0; i < g_context && getline(&line, &s, file) != -1; i++) {
                if (grep_strstr(line, pattern))
                    i = -1;
                print_line(name, ++lnum, line, pattern);
            }

            context_count = 0;
        } else if (context_count < g_context) {
            context[context_count++] = strdup(line);
        } else {
            free(context[0]);
            for (int i = 0; i < g_context - 1; i++)
                context[i] = context[i + 1];
            context[g_context - 1] = strdup(line);
        }
    }

    for (int i = 0; i < context_count; i++)
        free(context[i]);
    free(context);
    free(line);
}

void grep_file_noctx(char *name, FILE *file, char *pattern) {
    char *line = NULL;
    size_t s;

    for (int lnum = 0; getline(&line, &s, file) != -1; lnum++) {
        if (grep_strstr(line, pattern) != NULL)
            print_line(name, lnum, line, pattern);
    }
    free(line);
}

void grep_file_all(char *name, FILE *file, char *pattern) {
    char *line = NULL;
    size_t s;

    for (int lnum = 0; getline(&line, &s, file) != -1; lnum++)
        print_line(name, lnum, line, pattern);
    free(line);
}

void show_help(void) {
    puts(GREP_USAGE
        "Options:\n"
        "  -c    force color output\n"
        "  -f    print all the file lines\n"
        "  -h    display this help message\n"
        "  -H    print the file name\n"
        "  -i    Ignore case in patterns\n"
        "  -n    print line numbers\n"
        "  -NUM  show NUM lines of context\n"
    );
}

char **parse_args(int argc, char **argv) {
    g_context = 0;
    g_flags = 0;

    int file_count = 0;
    char **args = malloc(argc * sizeof(char *));

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            args[file_count++] = argv[i];
            continue;
        }
        switch (argv[i][1]) {
            case 'c':
                g_flags |= GREP_COLOR;
                break;
            case 'f':
                g_flags |= GREP_ALLLN;
                break;
            case 'h':
                show_help();
                exit(0);
            case 'H':
                g_flags |= GREP_FNAME;
                break;
            case 'i':
                g_flags |= GREP_ICASE;
                break;
            case 'n':
                g_flags |= GREP_LINES;
                break;
            case '0' ... '9':
                g_context = atoi(argv[i] + 1);
                break;
            case '-':
                fprintf(stderr, "grep: unrecognized option -- '%s'\n", argv[i]);
                fputs(GREP_USAGE, stderr);
                free(args);
                exit(1);
            default:
                fprintf(stderr, "grep: invalid option -- '%c'\n", argv[i][1]);
                fputs(GREP_USAGE, stderr);
                free(args);
                exit(1);
        }
    }

    if (file_count == 0) {
        fputs(GREP_USAGE, stderr);
        free(args);
        exit(1);
    }

    g_flags |= file_count > 2 ? GREP_FNAME : 0;
    g_flags |= isatty(1) ? GREP_COLOR : 0;

    args[file_count] = NULL;

    return args;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs(GREP_USAGE, stderr);
        return 1;
    }

    void (*grep_file)(char *, FILE *, char *);
    char **args = parse_args(argc, argv);

    if (g_flags & GREP_ALLLN)
        grep_file = grep_file_all;
    else if (g_context)
        grep_file = grep_file_ctx;
    else
        grep_file = grep_file_noctx;

    if (args[1] == NULL) {
        grep_file("stdin", stdin, args[0]);
        return 0;
    }

    for (int i = 1; args[i]; i++) {
        FILE *file = fopen(args[i], "r");
        if (file == NULL) {
            fprintf(stderr, "grep: %s: %m\n", args[i]);
            free(args);
            return 1;
        }
        grep_file(args[i], file, args[0]);
        fclose(file);
    }

    free(args);
    return 0;
}
