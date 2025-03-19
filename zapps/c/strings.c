/*****************************************************************************\
|   === strings.c : 2025 ===                                                  |
|                                                                             |
|    One-command string manipulation tool (upper, rev...)          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/arp.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

int g_arg;

void cmd_strings(FILE *file) {
    int first = -1;
    int c;

    for (int j = 0; (c = fgetc(file)) != EOF; j++) {
        if (isprint(c)) {
            if (first == -1)
                first = j;
        } else if (first != -1) {
            if (j - first >= g_arg) {
                fseek(file, first, SEEK_SET);
                for (int i = first; i < j; i++)
                    putchar(fgetc(file));
                putchar('\n');
                fgetc(file);
            }
            first = -1;
        }
    }
}

void cmd_upper(FILE *file) {
    int c;

    while ((c = fgetc(file)) != EOF)
        putchar(toupper(c));
}

void cmd_lower(FILE *file) {
    int c;

    while ((c = fgetc(file)) != EOF)
        putchar(tolower(c));
}

void cmd_rev(FILE *file) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        for (ssize_t i = read - 1; i >= 0; i--) {
            if (line[i] == '\n')
                continue;
            putchar(line[i]);
        }
        putchar('\n');
    }

    free(line);
}

int main(int argc, char **argv) {
    void (*cmd)(FILE *);

    arp_init("[-LUR -n %] [file1] [file2] ...", ARP_FNOMAX);

    arp_register('l', ARP_STANDARD, "convert to lowercase");
    arp_register('n', ARP_NEXT_INT, "strings size (default: 4)");
    arp_register('r', ARP_STANDARD, "reverse lines of text");
    arp_register('u', ARP_STANDARD, "convert to uppercase");
    arp_conflict("lurn,urn,rn");

    arp_set_ver("strings", NULL);

    if (arp_parse(argc, argv))
        return 1;

    if (arp_isset('l'))
        cmd = cmd_lower;
    else if (arp_isset('u'))
        cmd = cmd_upper;
    else if (arp_isset('r'))
        cmd = cmd_rev;
    else {
        g_arg = arp_isset('n') ? arp_get_int('n') : 4;
        cmd = cmd_strings;
    }

    if (arp_file_count() == 0) {
        cmd(stdin);
        return 0;
    }

    const char *path;
    while ((path = arp_file_next())) {
        FILE *file = fopen(path, "r");

        if (file == NULL) {
            fprintf(stderr, "%s: %s: %m\n", argv[0], path);
            return 1;
        }

        cmd(file);
        fclose(file);
    }

    return 0;
}
