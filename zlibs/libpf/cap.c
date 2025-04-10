/*****************************************************************************\
|   === cap.c : 2025 ===                                                      |
|                                                                             |
|    Command line argument parsing library                         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/cap.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define CAP_INFO "Try '%s -h' for more information.\n"

#define CAP_NUMMAX 0x7fffffff
#define CAP_VINDEX 0

#define CAP_ISVALID(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '%')
#define CAP_UNINIT ((void) fprintf(stderr, "cap: library not initialized\n"))

struct cap_option {
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
    struct cap_option *options;
    const char  *conflict;

    const char  *version_name;
    const char  *version;

    const char  *usage;
    const char  *name;

    const char **files;
    int    file_limits;
} *g_cap = NULL;

/********************************
 *                             *
 *   Init and Fini Functions   *
 *                             *
********************************/

static void cap_fini(void) {
    if (g_cap == NULL)
        return;

    free(g_cap->options);
    free(g_cap->files);
    free(g_cap);

    g_cap = NULL;
}

int cap_init(char *usage, unsigned max_files) {
    g_cap = calloc(1, sizeof(*g_cap));

    g_cap->options = calloc(127, sizeof(struct cap_option));
    g_cap->file_limits = max_files;
    g_cap->conflict = NULL;
    g_cap->version = NULL;
    g_cap->usage = usage;
    g_cap->files = NULL;

    cap_register('h', CAP_STANDARD, "show this help message");

    atexit(cap_fini);
    return 0;
}

void cap_set_ver(const char *name, const char *version) {
    if (g_cap == NULL)
        return CAP_UNINIT;
    g_cap->version_name = name;
    g_cap->version = version;
}

/********************************
 *                             *
 *   Help Message Generation   *
 *                             *
********************************/

static void print_help_line(struct cap_option *opt) {
    char letter;

    if (opt->c == 0 || opt->desc == NULL)
        return;

    switch (opt->flag) {
        case CAP_NEXT_STR:
            letter = '~';
            break;
        case CAP_NEXT_INT:
            letter = '%';
            break;
        case CAP_ANYNUMBR:
            printf("  -%%%%%%  %s\n", opt->desc);
            return;
        default:
            letter = ' ';
            break;
    }

    printf("  -%c %c  %s\n", opt->c, letter, opt->desc);
}

void cap_print_help(void) {
    if (g_cap == NULL)
        return CAP_UNINIT;

    printf("Usage: %s %s\nOptions:\n", g_cap->name, g_cap->usage);

    for (int i = 'a'; i <= 'z'; i++) {
        print_help_line(g_cap->options + (i - 'a' + 'A'));
        print_help_line(g_cap->options + i);
    }
    print_help_line(g_cap->options + '%');
}

static void cap_print_version(void) {
    if (g_cap == NULL)
        return CAP_UNINIT;

    fputs(g_cap->name, stdout);
    if (g_cap->version_name && strcmp(g_cap->version_name, g_cap->name) != 0)
        printf(" (aka %s)", g_cap->version_name);
    if (g_cap->version)
        printf(" version %s\n", g_cap->version);
    else
        printf(" - unknown version\n");
}

/***********************************
 *                                *
 *   Argument Register Function   *
 *                                *
***********************************/

int cap_register(char c, int flag, const char *desc) {
    if (g_cap == NULL)
        return (CAP_UNINIT, 1);

    if (!CAP_ISVALID(c))
        fprintf(stderr, "cap: character x%02x is not a valid option\n", c);
    else if (c == '%' && flag != CAP_ANYNUMBR)
        fprintf(stderr, "cap: flag CAP_ANYNUMBR expected for option '%c'\n", c);
    else if (c != '%' && flag == CAP_ANYNUMBR)
        fprintf(stderr, "cap: flag CAP_ANYNUMBR is reserved for option '%%'\n");

    else {
        g_cap->options[(int) c].c = c;
        g_cap->options[(int) c].desc = desc;
        g_cap->options[(int) c].flag = flag;
        return 0;
    }

    return 1;
}

int cap_conflict(const char *conflict) {
    if (g_cap == NULL)
        return (CAP_UNINIT, 1);

    // check if string represents a valid option
    for (int i = 0; conflict[i]; i++) {
        if (!CAP_ISVALID(conflict[i]) && conflict[i] != ',') {
            fprintf(stderr, "cap: character x%02x is not a valid option\n", conflict[i]);
            return 1;
        }
    }
    g_cap->conflict = conflict;
    return 0;
}

/**********************************
 *                               *
 *   Argument Parsing Function   *
 *                               *
**********************************/

static int cap_atoi(const char *str, int *value) {
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
        if (res > CAP_NUMMAX / base)
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

static int cap_check_conflicts(void) {
    char c;

    if (g_cap->conflict == NULL)
        return 0;

    for (int i = 0; g_cap->conflict[i];) {
        while (g_cap->conflict[i] == ',')
            i++;

        if (g_cap->conflict[i] == '\0')
            break;

        c = g_cap->conflict[i++];

        if (g_cap->options[(int) c].count == 0) {
            while (g_cap->conflict[i] && g_cap->conflict[i] != ',')
                i++;
            continue;
        }

        for (; g_cap->conflict[i] && g_cap->conflict[i] != ','; i++) {
            if (g_cap->options[(int) g_cap->conflict[i]].count == 0)
                continue;
            fprintf(stderr, "%s: incompatible options -- '%c' and '%c'\n" CAP_INFO, g_cap->name, c,
                    g_cap->conflict[i], g_cap->name);
            return 1;
        }
    }

    return 0;
}

int cap_parse(int argc, char **argv) {
    if (g_cap == NULL)
        return (CAP_UNINIT, 1);

    int files_index = 0;

    g_cap->files = calloc(argc + 1, sizeof(char *));
    g_cap->name = argv[0];

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            g_cap->files[files_index++] = argv[i];
            continue;
        }

        if (argv[i][1] == '\0') {
            fprintf(stderr, "Usage: %s %s\n" CAP_INFO, g_cap->name, g_cap->usage, g_cap->name);
            return 1;
        }

        for (int j = 1; argv[i][j]; j++) {
            int c = argv[i][j];

            if (c == '-') {
                if (j == 1) {
                    if (strcmp(argv[i], "--") == 0) {
                        for (i++; i < argc; i++)
                            g_cap->files[files_index++] = argv[i];
                        break;
                    }
                    if (strcmp(argv[i], "--help") == 0) {
                        g_cap->options['h'].count++;
                        break;
                    }
                    if (strcmp(argv[i], "--version") == 0) {
                        cap_print_version();
                        exit(0);
                    }
                }
                fprintf(stderr, "%s: unrecognized option '%s'\n" CAP_INFO,
                        g_cap->name, argv[i], g_cap->name);
                return 1;
            }


            if (!isprint(c)) {
                fprintf(stderr, "Usage: %s %s\n" CAP_INFO, g_cap->name, g_cap->usage, g_cap->name);
                return 1;
            }

            if (isdigit(c) && g_cap->options['%'].c) {
                if (g_cap->options['%'].count > 1) {
                    fprintf(stderr, "%s: numeric argument specified multiple times\n" CAP_INFO,
                            g_cap->name, g_cap->name);
                    return 1;
                }

                g_cap->options['%'].count++;
                int value = 0;

                while (isdigit(argv[i][j])) {
                    if (value > CAP_NUMMAX / 10) {
                        fprintf(stderr, "%s: numeric argument too large\n" CAP_INFO,
                                g_cap->name, g_cap->name);
                        return 1;
                    }
                    value = value * 10 + argv[i][j++] - '0';
                }
                j--;

                g_cap->options['%'].value.n = value;

                continue;
            }

            if (g_cap->options[c].c == 0) {
                fprintf(stderr, "%s: invalid option -- '%c'\n" CAP_INFO,
                        g_cap->name, c, g_cap->name);
                return 1;
            }

            g_cap->options[c].count++;

            if (g_cap->options[c].flag != CAP_NEXT_STR && g_cap->options[c].flag != CAP_NEXT_INT) {
                g_cap->options[c].value.n = 1;
                continue;
            }

            if (g_cap->options[c].count > 1) {
                fprintf(stderr, "%s: option '%c' specified multiple times\n" CAP_INFO,
                        g_cap->name, c, g_cap->name);
                return 1;
            }

            if (argv[i][j + 1] != '\0' || i + 1 == argc) {
                fprintf(stderr, "%s: option '%c' requires an argument\n" CAP_INFO,
                        g_cap->name, c, g_cap->name);
                return 1;
            }

            if (g_cap->options[c].flag == CAP_NEXT_STR) {
                g_cap->options[c].value.str = argv[++i];
                break;
            }

            // CAP_NEXT_INT
            if (cap_atoi(argv[++i], &g_cap->options[c].value.n)) {
                fprintf(stderr, "%s: option '%c' requires an numeric argument\n" CAP_INFO,
                        g_cap->name, c, g_cap->name);
                return 1;
            }

            break;
        }
    }

    if (g_cap->options['h'].count) {
        cap_print_help();
        exit(0);
    }

    int file_max =  g_cap->file_limits & 0x00ffffff;
    int file_min = (g_cap->file_limits & 0xff000000) >> 24;

    if (file_max < file_min)
        file_max = file_min;

    if (file_max != CAP_FNOMAX && files_index > file_max) {
        fprintf(stderr, "%s: too many arguments\n" CAP_INFO, g_cap->name, g_cap->name);
        return 1;
    }

    if (file_min != 0 && files_index < file_min) {
        fprintf(stderr, "%s: too few arguments\n" CAP_INFO, g_cap->name, g_cap->name);
        return 1;
    }

    return cap_check_conflicts();
}

/********************************
 *                             *
 *   Option Getter Functions   *
 *                             *
********************************/

int cap_isset(char c) {
    if (g_cap == NULL)
        return (CAP_UNINIT, 0);

    if (c < 0 || c > 126)
        return 0;
    return g_cap->options[(int) c].count;
}

const char *cap_get_str(char c) {
    if (g_cap == NULL)
        return (CAP_UNINIT, NULL);

    if (c < 0 || c > 126)
        return NULL;
    if (g_cap->options[(int) c].count == 0)
        return NULL;
    if (g_cap->options[(int) c].flag != CAP_NEXT_STR)
        return NULL;
    return g_cap->options[(int) c].value.str;
}

int cap_get_int(char c) {
    if (g_cap == NULL)
        return (CAP_UNINIT, -1);

    if (c < 0 || c > 126)
        return -1;
    if (g_cap->options[(int) c].count == 0)
        return -1;
    if (g_cap->options[(int) c].flag != CAP_NEXT_INT)
        return -1;
    return g_cap->options[(int) c].value.n;
}

const char **cap_get_files(void) {
    if (g_cap == NULL)
        return (CAP_UNINIT, NULL);

    return g_cap->files;
}

int cap_file_count(void) {
    if (g_cap == NULL)
        return (CAP_UNINIT, 0);

    int i = 0;
    while (g_cap->files[i] != NULL)
        i++;
    return i;
}

const char *cap_file_next(void) {
    if (g_cap == NULL)
        return (CAP_UNINIT, NULL);

    static int i = 0;
    return g_cap->files[i++];
}
