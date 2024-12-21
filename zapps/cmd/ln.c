/*****************************************************************************\
|   === ln.c : 2024 ===                                                       |
|                                                                             |
|    Unix command implementation - create links                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <profan/filesys.h>
#include <profan.h>

#define LN_SHORT "Try 'ln -h' for more information\n"

typedef struct {
    char *source;
    char *target;

    uint32_t sector;
    uint8_t is_hard;
} ln_args_t;

// format: d<u8:disk_id>s<u24:sector_id>
uint32_t parse_sid(char *str) {
    int i, j;
    if (str[0] != 'd')
        return SID_NULL;
    for (i = 1; str[i] >= '0' && str[i] <= '9'; i++);
    if (str[i++] != 's' || i == 1)
        return SID_NULL;
    for (j = i; str[j] >= '0' && str[j] <= '9'; j++);
    if (str[j] != '\0' || j == i)
        return SID_NULL;

    return SID_FORMAT(atoi(str + 1), atoi(str + i));
}

ln_args_t *parse_args(int argc, char **argv) {
    ln_args_t *args = calloc(1, sizeof(ln_args_t));
    args->is_hard = 1;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--", 2) == 0) {
            fprintf(stderr, "ln: unrecognized option '%s'\n" LN_SHORT, argv[i]);
            return NULL;
        } else if (argv[i][0] == '-' && strlen(argv[i]) == 2) {
            if (argv[i][1] == 's') {
                args->is_hard = 0;
                continue;
            }

            if (argv[i][1] == 'h') {
                puts("Usages:\n"
                    "  ln [-s] <source> [target]\n"
                    "  ln -d d<disk>s<sector> <target>\n"
                    "Options:\n"
                    "  -d  Force links to a specific sector\n"
                    "  -h  Display this help message\n"
                    "  -s  Create a symbolic link (not supported)");
                exit(0);
            }

            if (argv[i][1] != 'd') {
                fprintf(stderr, "ln: invalid option -- '%c'\n" LN_SHORT, argv[i][1]);
                return NULL;
            }

            // argv[i][1] == 'd'

            if (args->sector)
                return (fputs("ln: too many sectors\n" LN_SHORT, stderr), NULL);
            if (!args->is_hard)
                return (fputs("ln: cannot specify sector with symbolic link\n" LN_SHORT, stderr), NULL);
            if (args->target)
                return (fputs("ln: cannot specify sector with target\n" LN_SHORT, stderr), NULL);
            if (i + 1 >= argc)
                return (fputs("ln: missing sector\n" LN_SHORT, stderr), NULL);
            args->sector = parse_sid(argv[++i]);
            if (IS_SID_NULL(args->sector))
                return (fprintf(stderr, "ln: invalid sector '%s'\n" LN_SHORT, argv[i]), NULL);
        } else {
            if (!args->sector && !args->source) {
                args->source = argv[i];
            } else if (!args->target) {
                args->target = argv[i];
            } else {
                fputs("ln: too many arguments\n" LN_SHORT, stderr);
                return NULL;
            }
        }
    }

    if (!args->sector && !args->source) {
        fputs("ln: missing source\n" LN_SHORT, stderr);
        return NULL;
    }

    if (args->sector && !args->target) {
        fputs("ln: missing target\n" LN_SHORT, stderr);
        return NULL;
    }

    return args;
}

int main(int argc, char **argv) {
    ln_args_t *args = parse_args(argc, argv);

    if (!args)
        return 1;

    if (!args->is_hard) {
        fputs("ln: symbolic links are not supported\n", stderr);
        return 1;
    }

    char *target_dir, *target_name;
    if (args->target) {
        args->target = profan_join_path(profan_wd_path, args->target);
        profan_sep_path(args->target, &target_dir, &target_name);
    } else {
        target_dir = strdup(profan_wd_path);
        profan_sep_path(args->source, NULL, &target_name);
    }

    if (args->source) {
        args->source = profan_join_path(profan_wd_path, args->source);
        args->sector = fu_path_to_sid(SID_ROOT, args->source);
    }

    uint32_t target_sid = fu_path_to_sid(SID_ROOT, target_dir);


    if (IS_SID_NULL(args->sector)) {
        fprintf(stderr, "ln: cannot access '%s': No such file or directory\n", args->source);
        return 1;
    }

    if (IS_SID_NULL(target_sid)) {
        fprintf(stderr, "ln: cannot access '%s': No such file or directory\n", target_dir);
        return 1;
    }

    if (!IS_SID_NULL(fu_path_to_sid(target_sid, target_name))) {
        fprintf(stderr, "ln: cannot create link '%s': Name already exists\n", target_name);
        return 1;
    }

    fu_add_to_dir(target_sid, args->sector, target_name);

    return 0;
}
