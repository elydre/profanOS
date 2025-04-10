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

#include <profan/cap.h>

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
    cap_init("[options] [str1] [str2] ...", CAP_FNOMAX);

    cap_register('e', CAP_STANDARD, "enable backslash interpretation");
    cap_register('n', CAP_STANDARD, "do not print the trailing newline");
    cap_register('s', CAP_STANDARD, "print stdin followed by arguments");

    if (cap_parse(argc, argv))
        return 1;

    int echo_e = cap_isset('e');

    if (cap_isset('s')) {
        char *stdin_buffer = read_stdin();
        if (echo_e) {
            echo_interpret(stdin_buffer);
        } else {
            fputs(stdin_buffer, stdout);
        }
        free(stdin_buffer);
    }

    const char *word;

    for (int i = 0; (word = cap_file_next()); i++) {
        if (i > 0)
            putchar(' ');

        if (echo_e)
            echo_interpret((char *) word);
        else
            fputs(word, stdout);
    }

    if (!cap_isset('n')) {
        putchar('\n');
    }

    return 0;
}
