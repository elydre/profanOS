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

// @LINK: libpf

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/cap.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char no_error;
    char link_only;
    char preview;
    char allow_dirs;
    char verbose;
} rm_options_t;

int remove_hard_link(uint32_t elem, char *path) {
    char *parent;

    profan_path_sep(path, &parent, NULL);

    uint32_t parent_sid = fu_path_to_sid(SID_ROOT, parent);
    free(parent);

    if (IS_SID_NULL(parent_sid)) {
        fprintf(stderr, "rm: cannot remove '%s': unreachable path\n", path);
        return 1;
    }

    return fu_remove_from_dir(parent_sid, elem);
}

int remove_elem(uint32_t elem, char *path, rm_options_t *options) {
    if (!options->link_only && !(fu_is_file(elem) || fu_is_dir(elem) || fu_is_fctf(elem))) {
        fprintf(stderr, "rm: cannot remove '%s': unknown element type\n", path);
        return 1;
    }

    // recursive remove directory
    if (fu_is_dir(elem) && !options->link_only) {
        if (!options->allow_dirs) {
            fprintf(stderr, "rm: cannot remove '%s': is a directory (use -r)\n", path);
            return 1;
        }

        if (options->verbose)
            printf("rm: going into directory '%s'\n", path);

        uint32_t *content;
        char **names;
        int count = fu_dir_get_content(elem, &content, &names);

        if (count < 0) {
            fprintf(stderr, "rm: cannot remove '%s': failed to get directory content\n", path);
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
        fprintf(stderr, "rm: cannot remove '%s': failed to delete container\n", path);
        return 1;
    }

    return 0;
}

rm_options_t *parse_options(int argc, char **argv) {
    cap_init("[options] <path1> [path2] ...", CAP_FNOMAX | CAP_FMIN(1));

    cap_register('f', CAP_STANDARD, "ignore nonexistent files");
    cap_register('l', CAP_STANDARD, "remove only the link");
    cap_register('p', CAP_STANDARD, "preview the removal");
    cap_register('r', CAP_STANDARD, "allow recursive removal");
    cap_register('R', CAP_STANDARD, NULL);
    cap_register('v', CAP_STANDARD, "explain what is being done");

    cap_conflict("pl");

    if (cap_parse(argc, argv))
        exit(1);

    rm_options_t *options = malloc(sizeof(rm_options_t));
    options->no_error   = cap_isset('f');
    options->link_only  = cap_isset('l');
    options->preview    = cap_isset('p');
    options->allow_dirs = cap_isset('r') || cap_isset('R');
    options->verbose    = cap_isset('v') || cap_isset('p');

    return options;
}

int main(int argc, char **argv) {
    rm_options_t *options = parse_options(argc, argv);
    int exit_code = -1;

    const char *file;

    while ((file = cap_file_next())) {
        char *path = profan_path_join(profan_wd_path, file);
        uint32_t elem = fu_path_to_sid(SID_ROOT, path);

        if (IS_SID_NULL(elem)) {
            if (!options->no_error) {
                fprintf(stderr, "rm: cannot remove '%s': unreachable path\n", path);
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

    free(options);
    return exit_code;
}
