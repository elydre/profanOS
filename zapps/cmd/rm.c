#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>

typedef struct {
    int help;
    int verbose;
    int link_only;
    int preview;
    char *path;
} rm_options_t;

void print_help(int full) {
    puts("Usage: rm [options] <path>");
    if (!full) {
        puts("Try 'rm -h' for more information.");
        return;
    }
    puts("Options:\n"
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
                printf("rm: invalid option -- '%s'\n", argv[i]);
                options->help = 1;
            }
        } else {
            if (options->path) {
                printf("rm: extra operand '%s'\n", argv[i]);
                options->help = 1;
            } else {
                options->path = argv[i];
            }
        }
    }
    return options;
}

int remove_hard_link(sid_t elem, char *path) {
    char *parent;
    char *name;

    fu_sep_path(path, &parent, &name);

    sid_t parent_sid = fu_path_to_sid(ROOT_SID, parent);
    free(parent);
    free(name);

    if (IS_NULL_SID(parent_sid)) {
        printf("rm: cannot remove '%s': Unreachable path\n", path);
        return 1;
    }

    return fu_remove_element_from_dir(parent_sid, elem);
}

int remove_elem(sid_t elem, char *path, rm_options_t *options) {
    if (!(fu_is_file(elem) || fu_is_dir(elem) || fu_is_fctf(elem))) {
        printf("rm: cannot remove '%s': Unknown element type\n", path);
        return 1;
    }

    // recursive remove directory
    if (fu_is_dir(elem)) {
        if (options->verbose)
            printf("rm: going into directory '%s'\n", path);

        sid_t *content;
        char **names;
        int count = fu_get_dir_content(elem, &content, &names);

        if (count < 0) {
            printf("rm: cannot remove '%s': Failed to get directory content\n", path);
            return 1;
        }
        if (count == 0) {
            return 0;
        }
        char *new_path;
        for (int i = 0; i < count; i++) {
            if (strcmp(names[i], ".") == 0 || strcmp(names[i], "..") == 0) {
                free(names[i]);
                continue;
            }

            new_path = assemble_path(path, names[i]);

            if (remove_elem(content[i], new_path, options))
                exit(1);

            free(new_path);
            free(names[i]);
        }
        free(content);
        free(names);
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
    if (c_fs_cnt_delete(c_fs_get_main(), elem)) {
        printf("rm: cannot remove '%s': Failed to delete container\n", path);
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        print_help(0);
        return 1;
    }

    rm_options_t *options = parse_options(argc, argv);

    if (options->help == 1) {
        print_help(options->help);
        free(options);
        return 1;
    }

    if (options->help == 2) {
        print_help(options->help);
        free(options);
        return 0;
    }

    if (!options->path) {
        printf("rm: missing operand\n");
        print_help(0);
        free(options);
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *path = assemble_path(pwd, options->path);

    sid_t elem = fu_path_to_sid(ROOT_SID, path);

    if (IS_NULL_SID(elem)) {
        printf("rm: cannot remove '%s': Unreachable path\n", path);
        free(options);
        free(path);
        return 1;
    }

    int exit_code = remove_elem(elem, path, options);

    free(options);
    free(path);
    return exit_code;
}
