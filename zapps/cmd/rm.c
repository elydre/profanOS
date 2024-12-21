/*****************************************************************************\
|   === rm.c : 2024 ===                                                       |
|                                                                             |
|    Unix command implementation - removes files and dirs          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define RM_USAGE "Usage: rm [options] <path>\n"
#define RM_INFO "Try 'rm -h' for more information.\n"

typedef struct {
    int help;
    int verbose;
    int link_only;
    int preview;
    char *path;
} rm_options_t;

void print_help(void) {
    puts(
        "Usage: rm [options] <path>\n"
        "Options:\n"
        "  -h   display this help and exit\n"
        "  -v   explain what is being done\n"
        "  -l   remove only the link\n"
        "  -p   do not remove anything, only preview"
    );
}

rm_options_t *parse_options(int argc, char **argv) {
    rm_options_t *options = malloc(sizeof(rm_options_t));
    options->help = 0;
    options->verbose = 0;
    options->link_only = 0;
    options->preview = 0;
    options->path = NULL;

    int i;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-h") == 0) {
                options->help = 2;
            } else if (strcmp(argv[i], "-v") == 0) {
                options->verbose = 1;
            } else if (strcmp(argv[i], "-l") == 0) {
                options->link_only = 1;
            } else if (strcmp(argv[i], "-p") == 0) {
                options->preview = 1;
                options->verbose = 1;
            } else {
                fprintf(stderr, "rm: Invalid option -- '%s'\n", argv[i]);
                options->help = 1;
            }
        } else {
            if (options->path) {
                fprintf(stderr, "rm: Extra operand '%s'\n", argv[i]);
                options->help = 1;
            } else {
                options->path = argv[i];
            }
        }
    }
    return options;
}

int remove_hard_link(uint32_t elem, char *path) {
    char *parent;

    profan_sep_path(path, &parent, NULL);

    uint32_t parent_sid = fu_path_to_sid(SID_ROOT, parent);
    free(parent);

    if (IS_SID_NULL(parent_sid)) {
        fprintf(stderr, "rm: Cannot remove '%s': Unreachable path\n", path);
        return 1;
    }

    return fu_remove_from_dir(parent_sid, elem);
}

int remove_elem(uint32_t elem, char *path, rm_options_t *options) {
    if (!options->link_only && !(fu_is_file(elem) || fu_is_dir(elem) || fu_is_fctf(elem))) {
        fprintf(stderr, "rm: Cannot remove '%s': Unknown element type\n", path);
        return 1;
    }

    // recursive remove directory
    if (fu_is_dir(elem) && !options->link_only) {
        if (options->verbose)
            printf("rm: Going into directory '%s'\n", path);

        uint32_t *content;
        char **names;
        int count = fu_get_dir_content(elem, &content, &names);

        if (count < 0) {
            fprintf(stderr, "rm: Cannot remove '%s': Failed to get directory content\n", path);
            return 1;
        }
        if (count == 0) {
            return 0;
        }
        char *new_path;
        for (int i = 0; i < count; i++) {
            if (strcmp(names[i], ".") == 0 || strcmp(names[i], "..") == 0) {
                profan_kfree(names[i]);
                continue;
            }

            new_path = profan_join_path(path, names[i]);

            if (remove_elem(content[i], new_path, options))
                exit(1);

            free(new_path);
            profan_kfree(names[i]);
        }
        profan_kfree(content);
        profan_kfree(names);
    }

    if (options->verbose)
        printf("rm: removing '%s'\n", path);
    if (options->preview)
        return 0;

    // remove link
    if (remove_hard_link(elem, path))
        return 1;
    if (options->link_only)
        return 0;

    // delete container
    if (syscall_fs_delete(NULL, elem)) {
        fprintf(stderr, "rm: Cannot remove '%s': Failed to delete container\n", path);
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs(RM_USAGE RM_INFO, stderr);
        return 1;
    }

    rm_options_t *options = parse_options(argc, argv);

    if (options->help == 1) {
        fputs(RM_USAGE, stderr);
        free(options);
        return 1;
    }

    if (options->help == 2) {
        print_help();
        free(options);
        return 0;
    }

    if (!options->path) {
        fputs("rm: Missing path\n" RM_INFO, stderr);
        free(options);
        return 1;
    }

    char *path = profan_join_path(profan_wd_path, options->path);
    uint32_t elem = fu_path_to_sid(SID_ROOT, path);

    if (IS_SID_NULL(elem)) {
        fprintf(stderr, "rm: Cannot remove '%s': Unreachable path\n", path);
        free(options);
        free(path);
        return 1;
    }

    int exit_code = remove_elem(elem, path, options);

    free(options);
    free(path);
    return exit_code;
}
