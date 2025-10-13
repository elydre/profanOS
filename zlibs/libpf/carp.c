/*****************************************************************************\
|   === carp.c : 2025 ===                                                     |
|                                                                             |
|    Command line argument parsing library                         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/carp.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define CARP_INFO "Try '%s -h' for more information.\n"

#define CARP_NUMMAX 0x7fffffff

#define CARP_ISVALID(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '%')
#define CARP_UNINIT ((void) fprintf(stderr, "carp: library not initialized\n"))

struct carp_option {
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
    struct carp_option *options;
    const char  *conflict;

    const char  *version_name;
    const char  *version;

    const char  *usage;
    const char  *name;

    const char **files;
    int    file_limits;
} *g_carp = NULL;

/********************************
 *                             *
 *   Init and Fini Functions   *
 *                             *
********************************/

static void carp_fini(void) {
    if (g_carp == NULL)
        return;

    free(g_carp->options);
    free(g_carp->files);
    free(g_carp);

    g_carp = NULL;
}

int carp_init(char *usage, unsigned max_files) {
    g_carp = calloc(1, sizeof(*g_carp));

    g_carp->options = calloc(127, sizeof(struct carp_option));
    g_carp->file_limits = max_files;
    g_carp->conflict = NULL;
    g_carp->version = NULL;
    g_carp->usage = usage;
    g_carp->files = NULL;

    carp_register('h', CARP_STANDARD, "show this help message");

    atexit(carp_fini);
    return 0;
}

void carp_set_ver(const char *name, const char *version) {
    if (g_carp == NULL)
        return CARP_UNINIT;
    g_carp->version_name = name;
    g_carp->version = version;
}

/********************************
 *                             *
 *   Help Message Generation   *
 *                             *
********************************/

static void print_help_line(struct carp_option *opt) {
    char letter;

    if (opt->c == 0 || opt->desc == NULL)
        return;

    switch (opt->flag) {
        case CARP_NEXT_STR:
            letter = '~';
            break;
        case CARP_NEXT_INT:
            letter = '%';
            break;
        case CARP_ANYNUMBR:
            printf("  -%%%%%%  %s\n", opt->desc);
            return;
        default:
            letter = ' ';
            break;
    }

    printf("  -%c %c  %s\n", opt->c, letter, opt->desc);
}

void carp_show_help(void) {
    if (g_carp == NULL)
        return CARP_UNINIT;

    printf("Usage: %s %s\nOptions:\n", g_carp->name, g_carp->usage);

    for (int i = 'a'; i <= 'z'; i++) {
        print_help_line(g_carp->options + (i - 'a' + 'A'));
        print_help_line(g_carp->options + i);
    }
    print_help_line(g_carp->options + '%');
}

void carp_show_usage(void) {
    if (g_carp == NULL)
        return CARP_UNINIT;

    fprintf(stderr, "Usage: %s %s\n" CARP_INFO, g_carp->name, g_carp->usage, g_carp->name);
}

void carp_show_info(void) {
    if (g_carp == NULL)
        return CARP_UNINIT;

    fprintf(stderr, CARP_INFO, g_carp->name);
}

void carp_show_version(void) {
    if (g_carp == NULL)
        return CARP_UNINIT;

    fputs(g_carp->name, stdout);
    if (g_carp->version_name && strcmp(g_carp->version_name, g_carp->name) != 0)
        printf(" (aka %s)", g_carp->version_name);
    if (g_carp->version)
        printf(" %s", g_carp->version);
    puts(" - profanOS command pack");
}

/***********************************
 *                                *
 *   Argument Register Function   *
 *                                *
***********************************/

int carp_register(char c, int flag, const char *desc) {
    if (g_carp == NULL)
        return (CARP_UNINIT, 1);

    if (!CARP_ISVALID(c))
        fprintf(stderr, "carp: character x%02x is not a valid option\n", c);
    else if (c == '%' && flag != CARP_ANYNUMBR)
        fprintf(stderr, "carp: flag CARP_ANYNUMBR expected for option '%c'\n", c);
    else if (c != '%' && flag == CARP_ANYNUMBR)
        fprintf(stderr, "carp: flag CARP_ANYNUMBR is reserved for option '%%'\n");

    else {
        g_carp->options[(int) c].c = c;
        g_carp->options[(int) c].desc = desc;
        g_carp->options[(int) c].flag = flag;
        return 0;
    }

    return 1;
}

int carp_conflict(const char *conflict) {
    if (g_carp == NULL)
        return (CARP_UNINIT, 1);

    // check if string represents a valid option
    for (int i = 0; conflict[i]; i++) {
        if (!CARP_ISVALID(conflict[i]) && conflict[i] != ',') {
            fprintf(stderr, "carp: character x%02x is not a valid option\n", conflict[i]);
            return 1;
        }
    }
    g_carp->conflict = conflict;
    return 0;
}

/**********************************
 *                               *
 *   Argument Parsing Function   *
 *                               *
**********************************/

static int carp_atoi(const char *str, int *value) {
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
        if (res > CARP_NUMMAX / base)
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

static int carp_check_conflicts(void) {
    char c;

    if (g_carp->conflict == NULL)
        return 0;

    for (int i = 0; g_carp->conflict[i];) {
        while (g_carp->conflict[i] == ',')
            i++;

        if (g_carp->conflict[i] == '\0')
            break;

        c = g_carp->conflict[i++];

        if (g_carp->options[(int) c].count == 0) {
            while (g_carp->conflict[i] && g_carp->conflict[i] != ',')
                i++;
            continue;
        }

        for (; g_carp->conflict[i] && g_carp->conflict[i] != ','; i++) {
            if (g_carp->options[(int) g_carp->conflict[i]].count == 0)
                continue;
            fprintf(stderr, "%s: incompatible options -- '%c' and '%c'\n" CARP_INFO, g_carp->name, c,
                    g_carp->conflict[i], g_carp->name);
            return 1;
        }
    }

    return 0;
}

int carp_parse(int argc, char **argv) {
    if (g_carp == NULL)
        return (CARP_UNINIT, 1);

    int files_index = 0;

    g_carp->files = calloc(argc + 1, sizeof(char *));
    g_carp->name = argv[0];

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            g_carp->files[files_index++] = argv[i];
            continue;
        }

        if (argv[i][1] == '\0') {
            carp_show_usage();
            return 1;
        }

        for (int j = 1; argv[i][j]; j++) {
            int c = argv[i][j];

            if (c == '-') {
                if (j == 1) {
                    if (strcmp(argv[i], "--") == 0) {
                        for (i++; i < argc; i++)
                            g_carp->files[files_index++] = argv[i];
                        break;
                    }
                    if (strcmp(argv[i], "--help") == 0) {
                        g_carp->options['h'].count++;
                        break;
                    }
                    if (strcmp(argv[i], "--version") == 0) {
                        carp_show_version();
                        exit(0);
                    }
                }
                fprintf(stderr, "%s: unrecognized option '%s'\n" CARP_INFO,
                        g_carp->name, argv[i], g_carp->name);
                return 1;
            }


            if (!isprint(c)) {
                carp_show_usage();
                return 1;
            }

            if (isdigit(c) && g_carp->options['%'].c) {
                if (g_carp->options['%'].count > 1) {
                    fprintf(stderr, "%s: numeric argument specified multiple times\n" CARP_INFO,
                            g_carp->name, g_carp->name);
                    return 1;
                }

                g_carp->options['%'].count++;
                int value = 0;

                while (isdigit(argv[i][j])) {
                    if (value > CARP_NUMMAX / 10) {
                        fprintf(stderr, "%s: numeric argument too large\n" CARP_INFO,
                                g_carp->name, g_carp->name);
                        return 1;
                    }
                    value = value * 10 + argv[i][j++] - '0';
                }
                j--;

                g_carp->options['%'].value.n = value;

                continue;
            }

            if (g_carp->options[c].c == 0) {
                fprintf(stderr, "%s: invalid option -- '%c'\n" CARP_INFO,
                        g_carp->name, c, g_carp->name);
                return 1;
            }

            g_carp->options[c].count++;

            if (g_carp->options[c].flag != CARP_NEXT_STR && g_carp->options[c].flag != CARP_NEXT_INT) {
                g_carp->options[c].value.n = 1;
                continue;
            }

            if (g_carp->options[c].count > 1) {
                fprintf(stderr, "%s: option '%c' specified multiple times\n" CARP_INFO,
                        g_carp->name, c, g_carp->name);
                return 1;
            }

            if (argv[i][j + 1] != '\0' || i + 1 == argc) {
                fprintf(stderr, "%s: option '%c' requires an argument\n" CARP_INFO,
                        g_carp->name, c, g_carp->name);
                return 1;
            }

            if (g_carp->options[c].flag == CARP_NEXT_STR) {
                g_carp->options[c].value.str = argv[++i];
                break;
            }

            // CARP_NEXT_INT
            if (carp_atoi(argv[++i], &g_carp->options[c].value.n)) {
                fprintf(stderr, "%s: option '%c' requires an numeric argument\n" CARP_INFO,
                        g_carp->name, c, g_carp->name);
                return 1;
            }

            break;
        }
    }

    if (g_carp->options['h'].count) {
        carp_show_help();
        exit(0);
    }

    int file_max =  g_carp->file_limits & 0x00ffffff;
    int file_min = (g_carp->file_limits & 0xff000000) >> 24;

    if (file_max < file_min)
        file_max = file_min;

    if (file_max != CARP_ANYNUMBR && files_index > file_max) {
        fprintf(stderr, "%s: too many arguments\n" CARP_INFO, g_carp->name, g_carp->name);
        return 1;
    }

    if (file_min != 0 && files_index < file_min) {
        fprintf(stderr, "%s: too few arguments\n" CARP_INFO, g_carp->name, g_carp->name);
        return 1;
    }

    return carp_check_conflicts();
}

/********************************
 *                             *
 *   Option Getter Functions   *
 *                             *
********************************/

int carp_isset(char c) {
    if (g_carp == NULL)
        return (CARP_UNINIT, 0);

    if (c < 0 || c > 126)
        return 0;
    return g_carp->options[(int) c].count;
}

const char *carp_get_str(char c) {
    if (g_carp == NULL)
        return (CARP_UNINIT, NULL);

    if (c < 0 || c > 126)
        return NULL;
    if (g_carp->options[(int) c].count == 0)
        return NULL;
    if (g_carp->options[(int) c].flag != CARP_NEXT_STR)
        return NULL;
    return g_carp->options[(int) c].value.str;
}

int carp_get_int(char c) {
    if (g_carp == NULL)
        return (CARP_UNINIT, -1);

    if (c < 0 || c > 126)
        return -1;
    if (g_carp->options[(int) c].count == 0)
        return -1;
    if (g_carp->options[(int) c].flag != CARP_NEXT_INT)
        return -1;
    return g_carp->options[(int) c].value.n;
}

const char **carp_get_files(void) {
    if (g_carp == NULL)
        return (CARP_UNINIT, NULL);

    return g_carp->files;
}

int carp_file_count(void) {
    if (g_carp == NULL)
        return (CARP_UNINIT, 0);

    int i = 0;
    while (g_carp->files[i] != NULL)
        i++;
    return i;
}

const char *carp_file_next(void) {
    if (g_carp == NULL)
        return (CARP_UNINIT, NULL);

    static int i = 0;
    return g_carp->files[i++];
}
