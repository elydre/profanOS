/*****************************************************************************\
|   === getopt.c : 2026 ===                                                   |
|                                                                             |
|    Implementation of getopt functions from libC                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Based on the musl libc implementation                         `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <profan.h>
#include <limits.h>
#include <stdio.h>

int opterr = 1;     // if error message should be printed
int optind = 1;     // index into parent argv vector
int optopt;         // character checked for validity
int optreset;       // reset getopt

char *optarg;       // argument associated with option

static int optpos;  // internal: offset in current argv element

int *__getoptind(void) {
    return &optind;
}

int *__getopterr(void) {
    return &opterr;
}

int *__getoptopt(void) {
    return &optopt;
}

int *__getoptreset(void) {
    return &optreset;
}

char **__getoptarg(void) {
    return &optarg;
}


int getopt(int argc, char *const *argv, const char *optstring) {
    int i;
    wchar_t c, d;
    int k, l;
    char *optchar;

    if (!optind || optreset) {
        optreset = 0;
        optpos = 0;
        optind = 1;
    }

    if (optind >= argc || !argv[optind])
        return -1;

    if (argv[optind][0] != '-') {
        if (optstring[0] != '-')
            return -1;
        optarg = argv[optind++];
        return 1;
    }

    if (!argv[optind][1])
        return -1;

    if (argv[optind][1] == '-' && !argv[optind][2])
        return optind++, -1;

    if (!optpos)
        optpos++;

    if ((k = mbtowc(&c, argv[optind]+optpos, MB_LEN_MAX)) < 0) {
        k = 1;
        c = 0xfffd; /* replacement char */
    }
    optchar = argv[optind]+optpos;
    optpos += k;

    if (!argv[optind][optpos]) {
        optind++;
        optpos = 0;
    }

    if (optstring[0] == '-' || optstring[0] == '+')
        optstring++;

    i = 0;
    d = 0;
    do {
        l = mbtowc(&d, optstring+i, MB_LEN_MAX);
        i += l > 0 ? l : 1;
    } while (l && d != c);

    if (d != c || c == ':') {
        optopt = c;
        if (optstring[0] != ':' && opterr)
            fprintf(stderr, "%s: unrecognized option: %c\n", argv[0], *optchar);
        return '?';
    }

    if (optstring[i] == ':') {
        optarg = 0;
        if (optstring[i+1] != ':' || optpos) {
            optarg = argv[optind++];
            if (optpos)
                optarg += optpos;
            optpos = 0;
        }

        if (optind > argc) {
            optopt = c;
            if (optstring[0] == ':')
                return ':';
            if (opterr)
                fprintf(stderr, "%s: option requires an argument: %c\n", argv[0], *optchar);
            return '?';
        }
    }
    return c;
}


static void permute(char *const *argv, int dest, int src) {
    char **av = (char **)argv;
    char *tmp = av[src];
    int i;
    for (i = src; i > dest; i--)
        av[i] = av[i - 1];
    av[dest] = tmp;
}

static int getopt_long_core(int argc, char *const *argv, const char *optstring,
                                const struct option *longopts, int *idx, int longonly)
{
    optarg = 0;
    if (longopts && argv[optind][0] == '-' && (
                (longonly && argv[optind][1] && argv[optind][1] != '-') ||
                (argv[optind][1] == '-' && argv[optind][2])
    )) {
        char *arg, *opt, *start = argv[optind]+1;
        int i, cnt, match;

        int colon = optstring[optstring[0] == '+' || optstring[0] == '-'] == ':';

        for (cnt = i = 0; longopts[i].name; i++) {
            const char *name = longopts[i].name;
            opt = start;

            if (*opt == '-')
                opt++;

            while (*opt && *opt != '=' && *opt == *name) {
                name++;
                opt++;
            }

            if (*opt && *opt != '=')
                continue;

            arg = opt;
            match = i;

            if (!*name) {
                cnt = 1;
                break;
            }

            cnt++;
        }

        if (cnt == 1 && longonly && arg - start == mblen(start, MB_LEN_MAX)) {
            int l = arg - start;
            for (i = 0; optstring[i]; i++) {
                int j;
                for (j = 0; j < l && start[j] == optstring[i+j]; j++);
                if (j == l) {
                    cnt++;
                    break;
                }
            }
        }

        if (cnt==1) {
            i = match;
            opt = arg;
            optind++;

            if (*opt == '=') {
                if (!longopts[i].has_arg) {
                    optopt = longopts[i].val;
                    if (colon || !opterr)
                        return '?';
                    fprintf(stderr, "%s: option does not take an argument: %s\n",
                                argv[0], longopts[i].name);
                    return '?';
                }
                optarg = opt+1;
            }

            else if (longopts[i].has_arg == required_argument) {
                if (!(optarg = argv[optind])) {
                    optopt = longopts[i].val;
                    if (colon)
                        return ':';
                    if (!opterr)
                        return '?';

                    fprintf(stderr, "%s: option requires an argument: %s\n",
                                argv[0], longopts[i].name);
                    return '?';
                }
                optind++;
            }

            if (idx)
                *idx = i;

            if (longopts[i].flag) {
                *longopts[i].flag = longopts[i].val;
                return 0;
            }

            return longopts[i].val;
        }

        if (argv[optind][1] == '-') {
            optopt = 0;
            if (!colon && opterr)
                fprintf(stderr, "%s: %s: %s\n", argv[0],
                    cnt ? "option is ambiguous" : "unrecognized option",
                    argv[optind] + 2);

            optind++;
            return '?';
        }
    }

    return getopt(argc, argv, optstring);
}

static int getopt_long_func(int argc, char *const *argv, const char *optstring,
                                const struct option *longopts, int *idx, int longonly)
{
    int ret, skipped, resumed;
    if (!optind || optreset) {
        optreset = 0;
        optpos = 0;
        optind = 1;
    }

    if (optind >= argc || !argv[optind])
        return -1;

    skipped = optind;

    if (optstring[0] != '+' && optstring[0] != '-') {
        int i;
        for (i = optind; ; i++) {
            if (i >= argc || !argv[i])
                return -1;
            if (argv[i][0] == '-' && argv[i][1])
                break;
        }
        optind = i;
    }

    resumed = optind;
    ret = getopt_long_core(argc, argv, optstring, longopts, idx, longonly);

    if (resumed > skipped) {
        int i, cnt = optind-resumed;
        for (i = 0; i < cnt; i++)
            permute(argv, skipped, optind - 1);
        optind = skipped + cnt;
    }

    return ret;
}

int getopt_long(int argc, char *const *argv, const char *optstring, const struct option *longopts, int *idx) {
    return getopt_long_func(argc, argv, optstring, longopts, idx, 0);
}

int getopt_long_only(int argc, char *const *argv, const char *optstring, const struct option *longopts, int *idx) {
    return getopt_long_func(argc, argv, optstring, longopts, idx, 1);
}
