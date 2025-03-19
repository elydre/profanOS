/*****************************************************************************\
|   === arp.c : 2025 ===                                                      |
|                                                                             |
|    Command line argument parsing library                         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/arp.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define ARP_INFO "Try '%s -h' for more information.\n"

#define ARP_NUMMAX 0x7fffffff
#define ARP_VINDEX 0

#define ARP_ISVALID(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '%')
#define ARP_UNINIT ((void) fprintf(stderr, "arp: library not initialized\n"))

struct arp_option {
    char c;
    const char *desc;
    int flag;

    int count;

    union {
        const char *str;
        int n;
    } value;
};

struct {
    struct arp_option *options;
    const char  *conflict;

    const char  *version_name;
    const char  *version;

    const char  *usage;
    const char  *name;

    const char **files;
    int    file_limits;
} *g_arp = NULL;

/********************************
 *                             *
 *   Init and Fini Functions   *
 *                             *
********************************/

static void arp_fini(void) {
    if (g_arp == NULL)
        return;

    free(g_arp->options);
    free(g_arp->files);
    free(g_arp);

    g_arp = NULL;
}

int arp_init(char *usage, unsigned max_files) {
    g_arp = calloc(1, sizeof(*g_arp));

    g_arp->options = calloc(127, sizeof(struct arp_option));
    g_arp->file_limits = max_files;
    g_arp->conflict = NULL;
    g_arp->version = NULL;
    g_arp->usage = usage;
    g_arp->files = NULL;

    arp_register('h', ARP_STANDARD, "show this help message");

    atexit(arp_fini);
    return 0;
}

void arp_set_ver(const char *name, const char *version) {
    if (g_arp == NULL)
        return ARP_UNINIT;
    g_arp->version_name = name;
    g_arp->version = version;
}

/********************************
 *                             *
 *   Help Message Generation   *
 *                             *
********************************/

static void print_help_line(struct arp_option *opt) {
    char letter;

    if (opt->c == 0 || opt->desc == NULL)
        return;

    switch (opt->flag) {
        case ARP_NEXT_STR:
            letter = '~';
            break;
        case ARP_NEXT_INT:
            letter = '%';
            break;
        case ARP_ANYNUMBR:
            printf("  -%%%%%%  %s\n", opt->desc);
            return;
        default:
            letter = ' ';
            break;
    }

    printf("  -%c %c  %s\n", opt->c, letter, opt->desc);
}

void arp_print_help(void) {
    if (g_arp == NULL)
        return ARP_UNINIT;

    printf("Usage: %s %s\nOptions:\n", g_arp->name, g_arp->usage);

    for (int i = 'a'; i <= 'z'; i++) {
        print_help_line(g_arp->options + (i - 'a' + 'A'));
        print_help_line(g_arp->options + i);
    }
    print_help_line(g_arp->options + '%');
}

static void arp_print_version(void) {
    if (g_arp == NULL)
        return ARP_UNINIT;

    fputs(g_arp->name, stdout);
    if (g_arp->version_name && strcmp(g_arp->version_name, g_arp->name) != 0)
        printf(" (aka %s)", g_arp->version_name);
    if (g_arp->version)
        printf(" version %s\n", g_arp->version);
    else
        printf(" - unknown version\n");
}

/***********************************
 *                                *
 *   Argument Register Function   *
 *                                *
***********************************/

int arp_register(char c, int flag, const char *desc) {
    if (g_arp == NULL)
        return (ARP_UNINIT, 1);

    if (!ARP_ISVALID(c))
        fprintf(stderr, "arp: character x%02x is not a valid option\n", c);
    else if (c == '%' && flag != ARP_ANYNUMBR)
        fprintf(stderr, "arp: flag ARP_ANYNUMBR expected for option '%c'\n", c);
    else if (c != '%' && flag == ARP_ANYNUMBR)
        fprintf(stderr, "arp: flag ARP_ANYNUMBR is reserved for option '%%'\n");

    else {
        g_arp->options[(int) c].c = c;
        g_arp->options[(int) c].desc = desc;
        g_arp->options[(int) c].flag = flag;
        return 0;
    }

    return 1;
}

int arp_conflict(const char *conflict) {
    if (g_arp == NULL)
        return (ARP_UNINIT, 1);

    // check if string represents a valid option
    for (int i = 0; conflict[i]; i++) {
        if (!ARP_ISVALID(conflict[i]) && conflict[i] != ',') {
            fprintf(stderr, "arp: character x%02x is not a valid option\n", conflict[i]);
            return 1;
        }
    }
    g_arp->conflict = conflict;
    return 0;
}

/**********************************
 *                               *
 *   Argument Parsing Function   *
 *                               *
**********************************/

static int arp_atoi(const char *str, int *value) {
    int sign, found, base = 0;
    int res = 0;

    char *base_str;

    if (str[0] == '\0')
        return 1;

    if (str[0] == '-') {
        sign = 1;
        str++;
    } else {
        sign = 0;
    }

    if (str[0] == '0' && str[1] == 'x') {
        base_str = "0123456789abcdef";
        base = 16;
        str += 2;
    } else if (str[0] == '0' && str[1] == 'b') {
        base_str = "01";
        base = 2;
        str += 2;
    } else {
        base_str = "0123456789";
        base = 10;
    }

    for (int i = 0; base && str[i] != '\0'; i++) {
        found = 0;
        if (res > ARP_NUMMAX / base)
            return 1;
        for (int j = 0; base_str[j] != '\0'; j++) {
            if (tolower(str[i]) == base_str[j]) {
                res *= base;
                res += j;
                found = 1;
                break;
            }
        }
        if (!found)
            return 1;
    }

    if (sign)
        res = -res;

    if (value != NULL)
        *value = res;

    return 0;
}

static int arp_check_conflicts(void) {
    char c;

    if (g_arp->conflict == NULL)
        return 0;

    for (int i = 0; g_arp->conflict[i];) {
        while (g_arp->conflict[i] == ',')
            i++;

        if (g_arp->conflict[i] == '\0')
            break;

        c = g_arp->conflict[i++];

        if (g_arp->options[(int) c].count == 0) {
            while (g_arp->conflict[i] && g_arp->conflict[i] != ',')
                i++;
            continue;
        }

        for (; g_arp->conflict[i] && g_arp->conflict[i] != ','; i++) {
            if (g_arp->options[(int) g_arp->conflict[i]].count == 0)
                continue;
            fprintf(stderr, "%s: incompatible options -- '%c' and '%c'\n" ARP_INFO, g_arp->name, c,
                    g_arp->conflict[i], g_arp->name);
            return 1;
        }
    }

    return 0;
}

int arp_parse(int argc, char **argv) {
    if (g_arp == NULL)
        return (ARP_UNINIT, 1);

    int files_index = 0;

    g_arp->files = calloc(argc + 1, sizeof(char *));
    g_arp->name = argv[0];

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            g_arp->files[files_index++] = argv[i];
            continue;
        }

        if (argv[i][1] == '\0') {
            fprintf(stderr, "Usage: %s %s\n" ARP_INFO, g_arp->name, g_arp->usage, g_arp->name);
            return 1;
        }

        for (int j = 1; argv[i][j]; j++) {
            int c = argv[i][j];

            if (c == '-') {
                if (j == 1) {
                    if (strcmp(argv[i], "--") == 0) {
                        for (i++; i < argc; i++)
                            g_arp->files[files_index++] = argv[i];
                        break;
                    }
                    if (strcmp(argv[i], "--help") == 0) {
                        g_arp->options['h'].count++;
                        break;
                    }
                    if (strcmp(argv[i], "--version") == 0) {
                        arp_print_version();
                        exit(0);
                    }
                }
                fprintf(stderr, "%s: unrecognized option '%s'\n" ARP_INFO,
                        g_arp->name, argv[i], g_arp->name);
                return 1;
            }


            if (!isprint(c)) {
                fprintf(stderr, "Usage: %s %s\n" ARP_INFO, g_arp->name, g_arp->usage, g_arp->name);
                return 1;
            }

            if (isdigit(c) && g_arp->options['%'].c) {
                if (g_arp->options['%'].count > 1) {
                    fprintf(stderr, "%s: numeric argument specified multiple times\n" ARP_INFO,
                            g_arp->name, g_arp->name);
                    return 1;
                }

                g_arp->options['%'].count++;
                int value = 0;

                while (isdigit(argv[i][j])) {
                    if (value > ARP_NUMMAX / 10) {
                        fprintf(stderr, "%s: numeric argument too large\n" ARP_INFO,
                                g_arp->name, g_arp->name);
                        return 1;
                    }
                    value = value * 10 + argv[i][j++] - '0';
                }
                j--;

                g_arp->options['%'].value.n = value;

                continue;
            }

            if (g_arp->options[c].c == 0) {
                fprintf(stderr, "%s: invalid option -- '%c'\n" ARP_INFO,
                        g_arp->name, c, g_arp->name);
                return 1;
            }

            g_arp->options[c].count++;

            if (g_arp->options[c].flag != ARP_NEXT_STR && g_arp->options[c].flag != ARP_NEXT_INT) {
                g_arp->options[c].value.n = 1;
                continue;
            }

            if (g_arp->options[c].count > 1) {
                fprintf(stderr, "%s: option '%c' specified multiple times\n" ARP_INFO,
                        g_arp->name, c, g_arp->name);
                return 1;
            }

            if (argv[i][j + 1] != '\0' || i + 1 == argc) {
                fprintf(stderr, "%s: option '%c' requires an argument\n" ARP_INFO,
                        g_arp->name, c, g_arp->name);
                return 1;
            }

            if (g_arp->options[c].flag == ARP_NEXT_STR) {
                g_arp->options[c].value.str = argv[++i];
                break;
            }

            // ARP_NEXT_INT
            if (arp_atoi(argv[++i], &g_arp->options[c].value.n)) {
                fprintf(stderr, "%s: option '%c' requires an numeric argument\n" ARP_INFO,
                        g_arp->name, c, g_arp->name);
                return 1;
            }

            break;
        }
    }

    if (g_arp->options['h'].count) {
        arp_print_help();
        exit(0);
    }

    int file_max =  g_arp->file_limits & 0x00ffffff;
    int file_min = (g_arp->file_limits & 0xff000000) >> 24;

    if (file_max < file_min)
        file_max = file_min;

    if (file_max != ARP_FNOMAX && files_index > file_max) {
        fprintf(stderr, "%s: too many arguments\n" ARP_INFO, g_arp->name, g_arp->name);
        return 1;
    }

    if (file_min != 0 && files_index < file_min) {
        fprintf(stderr, "%s: too few arguments\n" ARP_INFO, g_arp->name, g_arp->name);
        return 1;
    }

    return arp_check_conflicts();
}

/********************************
 *                             *
 *   Option Getter Functions   *
 *                             *
********************************/

int arp_isset(char c) {
    if (g_arp == NULL)
        return (ARP_UNINIT, 0);

    if (c < 0 || c > 126)
        return 0;
    return g_arp->options[(int) c].count;
}

const char *arp_get_str(char c) {
    if (g_arp == NULL)
        return (ARP_UNINIT, NULL);

    if (c < 0 || c > 126)
        return NULL;
    return g_arp->options[(int) c].value.str;
}

int arp_get_int(char c) {
    if (g_arp == NULL)
        return (ARP_UNINIT, -1);

    if (c < 0 || c > 126)
        return -1;
    return g_arp->options[(int) c].value.n;
}

const char **arp_get_files(void) {
    if (g_arp == NULL)
        return (ARP_UNINIT, NULL);

    return g_arp->files;
}

int arp_file_count(void) {
    if (g_arp == NULL)
        return (ARP_UNINIT, 0);

    int i = 0;
    while (g_arp->files[i] != NULL)
        i++;
    return i;
}

const char *arp_file_next(void) {
    if (g_arp == NULL)
        return (ARP_UNINIT, NULL);

    static int i = 0;
    return g_arp->files[i++];
}
