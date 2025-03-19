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

// @LINK: libpf

#include <profan/arp.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

#define GREP_COLOR 0x01
#define GREP_FNAME 0x02
#define GREP_LINES 0x04
#define GREP_ICASE 0x08
#define GREP_ALLLN 0x10
#define GREP_RECUR 0x20

void (*grep_file)(const char *, FILE *, const char *);

uint32_t g_flags;
int      g_context;

const char *grep_strstr(const char *haystack, const char *needle) {
    if (g_flags & GREP_ICASE) {
        for (const char *h = haystack; *h; h++) {
            const char *n = needle;
            for (const char *h2 = h; *n && tolower(*n) == tolower(*h2); n++, h2++);
            if (!*n)
                return h;
        }
    } else {
        for (const char *h = haystack; *h; h++) {
            const char *n = needle;
            for (const char *h2 = h; *n && *n == *h2; n++, h2++);
            if (!*n)
                return h;
        }
    }
    return NULL;
}

void print_with_color(const char *line, const char *pattern) {
    const char *pattern_start, *pattern_end;

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

void print_line(const char *name, int lnum, const char *line, const char *pattern) {
    if (g_flags & GREP_COLOR) {
        if (g_flags & GREP_FNAME)
            printf("\e[37m%s:\e[0m ", name);
        if (g_flags & GREP_LINES)
            printf("\e[32m%d\e[90m:\e[0m ", lnum + 1);
        print_with_color(line, pattern);
    } else {
        if (g_flags & GREP_FNAME)
            printf("%s: ", name);
        if (g_flags & GREP_LINES)
            printf("%d: ", lnum + 1);
        fputs(line, stdout);
    }
}

void grep_file_ctx(const char *name, FILE *file, const char *pattern) {
    char *line = NULL;
    size_t s;

    char **context = malloc(g_context * sizeof(char *));
    int context_count = 0;
    int first = 1;

    for (int lnum = 0; getline(&line, &s, file) != -1; lnum++) {
        if (grep_strstr(line, pattern) != NULL) {
            if (first) first = 0;
            else if (context_count == g_context) fputs("--\n", stdout);

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

void grep_file_noctx(const char *name, FILE *file, const char *pattern) {
    char *line = NULL;
    size_t s;

    for (int lnum = 0; getline(&line, &s, file) != -1; lnum++) {
        if (grep_strstr(line, pattern) != NULL)
            print_line(name, lnum, line, pattern);
    }
    free(line);
}

void grep_file_all(const char *name, FILE *file, const char *pattern) {
    char *line = NULL;
    size_t s;

    for (int lnum = 0; getline(&line, &s, file) != -1; lnum++)
        print_line(name, lnum, line, pattern);
    free(line);
}

void grep_dir(const char *path, const char *pattern) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "grep: %s: %m\n", path);
        return;
    }

    char *fullpath = malloc(PATH_MAX);

    char *path_join = path[strlen(path) - 1] == '/' ? "" : "/";

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.')
            continue;

        snprintf(fullpath, PATH_MAX, "%s%s%s", path, path_join, entry->d_name);

        FILE *file = fopen(fullpath, "r");

        if (file == NULL) {
            if (errno != EISDIR)
                fprintf(stderr, "grep: %s: %m\n", fullpath);
            else
                grep_dir(fullpath, pattern);
            continue;
        }

        grep_file(fullpath, file, pattern);
        fclose(file);
    }

    free(fullpath);
    closedir(dir);
}

void update_flags(int argc, char **argv) {
    arp_init("[options] <pattern> [file1] [file2] ...", ARP_FMIN(1) | ARP_FNOMAX);

    arp_register('c', ARP_STANDARD, "force color output");
    arp_register('f', ARP_STANDARD, "print all the file lines");
    arp_register('h', ARP_STANDARD, "display this help message");
    arp_register('H', ARP_STANDARD, "print the file name");
    arp_register('i', ARP_STANDARD, "Ignore case in patterns");
    arp_register('n', ARP_STANDARD, "print line numbers");
    arp_register('r', ARP_STANDARD, "search recursively");
    arp_register('z', ARP_STANDARD, "force no file name display");
    arp_register('%', ARP_ANYNUMBR, "show NUM lines of context");

    arp_conflict("f%,Hz");

    if (arp_parse(argc, argv))
        exit(1);

    g_context = 0;
    g_flags = 0;

    if (arp_isset('c'))
        g_flags |= GREP_COLOR;
    if (arp_isset('f'))
        g_flags |= GREP_ALLLN;
    if (arp_isset('H'))
        g_flags |= GREP_FNAME;
    if (arp_isset('i'))
        g_flags |= GREP_ICASE;
    if (arp_isset('n'))
        g_flags |= GREP_LINES;
    if (arp_isset('r'))
        g_flags |= GREP_RECUR | GREP_FNAME;
    if (arp_isset('%'))
        g_context = arp_get_int('%');

    g_flags |= arp_file_count() > 2 ? GREP_FNAME : 0;
    g_flags |= isatty(1) ? GREP_COLOR : 0;

    if (arp_isset('z'))
        g_flags &= ~GREP_FNAME;
}

int main(int argc, char **argv) {
    update_flags(argc, argv);

    if (g_flags & GREP_ALLLN)
        grep_file = grep_file_all;
    else if (g_context)
        grep_file = grep_file_ctx;
    else
        grep_file = grep_file_noctx;

    const char **args = arp_get_files();

    if (args[1] == NULL) {
        grep_file("stdin", stdin, args[0]);
        return 0;
    }

    for (int i = 1; args[i]; i++) {
        FILE *file = fopen(args[i], "r");
        if (file) {
            grep_file(args[i], file, args[0]);
            fclose(file);
            continue;
        }

        if (g_flags & GREP_RECUR && errno == EISDIR) {
            grep_dir(args[i], args[0]);
        } else {
            fprintf(stderr, "grep: %s: %m\n", args[i]);
            return 1;
        }
    }

    return 0;
}
