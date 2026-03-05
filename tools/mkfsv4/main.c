/*****************************************************************************\
|   === main.c : 2024 ===                                                     |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "butterfly.h"

uint32_t sector_data[SECTOR_SIZE / sizeof(uint32_t)];

typedef struct {
    char *input;
    char *output;
    int print_tree;
    int extract;
} mkfs_options_t;

#define TRY_HELP "Try 'mkfsv4 -h' for more information.\n"
#define CMD_USAGE "Usage: mkfsv4 [-t] [-x] <input> <output>\n"

void print_help(void) {
    fputs(CMD_USAGE
            "Options:\n"
            "  -h    show this help message\n"
            "  -t    print the root directory tree\n"
            "  -x    extract fs-image to directory\n"
            "Examples:\n"
            "  (dir -> fs-image)   mkfsv4 <base directory> <output file>\n"
            "  (fs-image -> dir)   mkfsv4 -x <input file> <output directory>\n",
        stdout);
}

int parse_args(int argc, char **argv, mkfs_options_t *options) {
    options->print_tree = 0;
    options->extract = 0;

    int arg_index = 1;

    while (arg_index < argc && argv[arg_index][0] == '-') {
        if (argv[arg_index][1] == '-') {
            fprintf(stderr, "mkfsv4: unrecognized option '%s'\n" TRY_HELP, argv[arg_index]);
            return 1;
        } else if (argv[arg_index][1] == '\0') {
            fputs(CMD_USAGE TRY_HELP, stderr);
            return 1;
        }
        for (size_t i = 1; argv[arg_index][i] != '\0'; i++) {
            switch (argv[arg_index][i]) {
                case 't':
                    options->print_tree = 1;
                    break;
                case 'x':
                    options->extract = 1;
                    break;
                case 'h':
                    print_help();
                    exit(0);
                default:
                    fprintf(stderr, "mkfsv4: invalid option -- '%c'\n" TRY_HELP, argv[arg_index][i]);
                    return 1;
            }
        }
        arg_index++;
    }

    if (argc - arg_index != 2) {
        fprintf(stderr, CMD_USAGE TRY_HELP);
        return -1;
    }

    options->input = argv[arg_index];
    options->output = argv[arg_index + 1];

    if (options->output[0] == '-') {
        fputs(CMD_USAGE TRY_HELP, stderr);
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) {
    mkfs_options_t options;

    if (parse_args(argc, argv, &options))
        return 1;


    if (options.extract) {
        vdisk_open(options.input);
        hio_dir_export(options.output);
    } else {
        vdisk_new(options.output);
        hio_dir_import(options.input);
    }

    if (options.print_tree)
        fu_draw_tree(SID_ROOT, 0);

    vdisk_close();
    return 0;
}
