/*****************************************************************************\
|   === uname.c : 2025 ===                                                    |
|                                                                             |
|    Unix command implementation - print system information        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <sys/utsname.h>

#include <stdlib.h>
#include <stdio.h>

#define UNAME_USAGE "Usage: uname [options]\n"
#define UNAME_HELP "Try 'uname -h' for more information.\n"

enum {
    PRINT_SYSNAME  = 0x01,
    PRINT_NODENAME = 0x02,
    PRINT_RELEASE  = 0x04,
    PRINT_VERSION  = 0x08,
    PRINT_MACHINE  = 0x10,
    PRINT_OS       = 0x20,
    PRINT_ALL      = 0x3f
};

void uname_help(void) {
    puts(UNAME_USAGE
        "Options:\n"
        "  -a   print all information\n"
        "  -s   print kernel name\n"
        "  -n   print network node hostname\n"
        "  -r   print kernel release\n"
        "  -v   print kernel version\n"
        "  -m   print machine hardware name\n"
        "  -o   print operating system name"
    );
}

void print_element(int val, int mask, const char *str) {
    static int space = 0;

    if (!(val & mask))
        return;

    if (space)
        putchar(' ');
    else
        space = 1;

    fputs(str, stdout);
}

int main(int argc, char **argv) {
    struct utsname u;
    int print_mask = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || argv[i][1] == '\0' || argv[i][1] == '-') {
            fputs(UNAME_USAGE UNAME_HELP, stderr);
            return 1;
        }

        for (int j = 1; argv[i][j]; j++) {
            switch (argv[i][j]) {
                case 'a':
                    print_mask |= PRINT_ALL;
                    break;
                case 'h':
                    uname_help();
                    return 0;
                case 'm':
                    print_mask |= PRINT_MACHINE;
                    break;
                case 'n':
                    print_mask |= PRINT_NODENAME;
                    break;
                case 'r':
                    print_mask |= PRINT_RELEASE;
                    break;
                case 's':
                    print_mask |= PRINT_SYSNAME;
                    break;
                case 'v':
                    print_mask |= PRINT_VERSION;
                    break;
                case 'o':
                    print_mask |= PRINT_OS;
                    break;
                default:
                    fprintf(stderr, "uname: unknown option -- %c\n" UNAME_HELP, argv[i][j]);
                    return 1;
            }
        }
    }

    if (!print_mask)
        print_mask = PRINT_SYSNAME;

    if (uname(&u) == -1) {
        perror("uname");
        return 1;
    }

    print_element(print_mask, PRINT_SYSNAME,  u.sysname);
    print_element(print_mask, PRINT_NODENAME, u.nodename);
    print_element(print_mask, PRINT_RELEASE,  u.release);
    print_element(print_mask, PRINT_VERSION,  u.version);
    print_element(print_mask, PRINT_MACHINE,  u.machine);
    print_element(print_mask, PRINT_OS,       "profanOS");

    putchar('\n');

    return 0;
}
