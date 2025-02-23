/*****************************************************************************\
|   === rm.c : 2024 ===                                                       |
|                                                                             |
|    Unix command implementation - removes files / directories     .pi0iq.    |
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
    char no_error;
    char help;
    char link_only;
    char preview;
    char dir_allowed;
    char verbose;
} rm_options_t;

void print_help(void) {
    puts(RM_USAGE
        "Options:\n"
        "  -f     ignore nonexistent files\n"
        "  -h     display this help and exit\n"
        "  -l     remove only the link\n"
        "  -p     preview the removal\n"
        "  -r -R  allow recursive removal\n"
        "  -v     explain what is being done"
    );
}

int remove_hard_link(uint32_t elem, char *path) {
    char *parent;

    profan_path_sep(path, &parent, NULL);

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
        if (!options->dir_allowed) {
            fprintf(stderr, "rm: Cannot remove '%s': Is a directory (use -r)\n", path);
            return 1;
        }

        if (options->verbose)
            printf("rm: Going into directory '%s'\n", path);

        uint32_t *content;
        char **names;
        int count = fu_dir_get_content(elem, &content, &names);

        if (count < 0) {
            fprintf(stderr, "rm: Cannot remove '%s': Failed to get directory content\n", path);
            return 1;
        }

        if (count == 0)
            return 0;

        char *new_path;
        for (int i = 0; i < count; i++) {
            if (strcmp(names[i], ".") == 0 || strcmp(names[i], "..") == 0) {
                profan_kfree(names[i]);
                continue;
            }

            new_path = profan_path_join(path, names[i]);

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

rm_options_t *parse_options(int argc, char **argv) {
    rm_options_t *options = calloc(1, sizeof(rm_options_t));

    for (int i = 1; i < argc && options->help != 1; i++) {
        if (argv[i][0] != '-')
            continue;
        if (argv[i][1] == '\0') {
            fputs("rm: Empty option detected\n", stderr);
            options->help = 1;
            break;
        }
        for (int j = 1; argv[i][j] && options->help != 1; j++) {
            switch (argv[i][j]) {
                case 'f':
                    options->no_error = 1;
                    break;
                case 'h':
                    options->help = 2;
                    break;
                case 'l':
                    options->link_only = 1;
                    break;
                case 'p':
                    options->preview = 1;
                    break;
                case 'R':
                case 'r':
                    options->dir_allowed = 1;
                    break;
                case 'v':
                    options->verbose = 1;
                    break;
                case '-':
                    fprintf(stderr, "rm: Unrecognized option -- %s\n", argv[i]);
                    options->help = 1;
                    break;
                default:
                    fprintf(stderr, "rm: Invalid option -- '%c'\n", argv[i][j]);
                    options->help = 1;
                    break;
            }
        }
    }

    return options;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs(RM_USAGE RM_INFO, stderr);
        return 1;
    }

    rm_options_t *options = parse_options(argc, argv);
    int exit_code = -1;

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

    for (int i = 1; i < argc && exit_code < 1; i++) {
        if (argv[i][0] == '-')
            continue;

        char *path = profan_path_join(profan_wd_path, argv[i]);
        uint32_t elem = fu_path_to_sid(SID_ROOT, path);

        if (IS_SID_NULL(elem)) {
            if (!options->no_error) {
                fprintf(stderr, "rm: Cannot remove '%s': Unreachable path\n", path);
                exit_code = 1;
                free(path);
                continue;
            }
            exit_code = 0;
        } else {
            exit_code = remove_elem(elem, path, options);
        }

        free(path);
    }

    if (exit_code == -1) {
        fputs("rm: No path provided\n" RM_USAGE, stderr);
        exit_code = 1;
    }

    free(options);
    return exit_code;
}
