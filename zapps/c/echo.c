/*****************************************************************************\
|   === echo.c : 2024 ===                                                     |
|                                                                             |
|    Unix command implementation - print arguments to stdoud       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/carp.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

void echo_interpret(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] != '\\') {
            putchar(str[i]);
            continue;
        }
        switch (str[++i]) {
            case '\\':
                putchar('\\');
                break;
            case 'a':
                putchar('\a');
                break;
            case 'b':
                putchar('\b');
                break;
            case 'c':
                return;
            case 'e':
                putchar('\e');
                break;
            case 'f':
                putchar('\f');
                break;
            case 'n':
                putchar('\n');
                break;
            case 'r':
                putchar('\r');
                break;
            case 't':
                putchar('\t');
                break;
            case 'v':
                putchar('\v');
                break;
            case '0':
                putchar(strtol(str + ++i, NULL, 8));
                while (str[i] >= '0' && str[i] <= '7')
                    i++;
                i--;
                break;
            case 'x':
                putchar(strtol(str + ++i, NULL, 16));
                while (isxdigit(str[i]))
                    i++;
                i--;
                break;
            default:
                putchar(str[i]);
                break;
        }
    }
}

char *read_stdin(void) {
    // read from stdin
    char *buffer = malloc(1024);
    int buffer_size = 0;
    int rcount = 0;

    while ((rcount = fread(buffer + buffer_size, 1, 1024, stdin)) > 0) {
        buffer = realloc(buffer, buffer_size + rcount + 1025);
        buffer_size += rcount;
    }

    buffer[buffer_size] = '\0';

    return buffer;
}

int main(int argc, char **argv) {
    carp_init("[options] [str1] [str2] ...", CARP_FNOMAX);

    carp_register('e', CARP_STANDARD, "enable backslash interpretation");
    carp_register('n', CARP_STANDARD, "do not print the trailing newline");
    carp_register('s', CARP_STANDARD, "print stdin followed by arguments");

    if (carp_parse(argc, argv))
        return 1;

    int echo_e = carp_isset('e');

    if (carp_isset('s')) {
        char *stdin_buffer = read_stdin();
        if (echo_e) {
            echo_interpret(stdin_buffer);
        } else {
            fputs(stdin_buffer, stdout);
        }
        free(stdin_buffer);
    }

    const char *word;

    for (int i = 0; (word = carp_file_next()); i++) {
        if (i > 0)
            putchar(' ');

        if (echo_e)
            echo_interpret((char *) word);
        else
            fputs(word, stdout);
    }

    if (!carp_isset('n')) {
        putchar('\n');
    }

    return 0;
}
