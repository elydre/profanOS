/*****************************************************************************\
|   === mod.c : 2024 ===                                                      |
|                                                                             |
|    Command to load and unload kernel modules                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan.h>


#define HELP_USAGE "Usage: mod <option> [args]\n"
#define HELP_HELP "Try 'mod -h' for more information.\n"

int is_file(char *path) {
    uint32_t sid = fu_path_to_sid(ROOT_SID, path);
    if (IS_SID_NULL(sid)) return 0;
    return fu_is_file(sid);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs(HELP_USAGE HELP_HELP, stderr);
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char option = *(argv[1] + 1);
    if (option == '-') {
        option = *(argv[1] + 2);
    }

    if (option == '\0') {
        fprintf(stderr, "mod: Invalid option -- '%s'\n" HELP_HELP, argv[1]);
        return 1;
    }

    if (option == 'h') {
        puts(HELP_USAGE
            "Available options:\n"
            "  -h              print this help message\n"
            "  -u <id>         unload a library\n"
            "  -l <id> <path>  load a library\n"
            "  -r <id> <path>  replace a library"
        );
        return 0;
    }

    if (option == 'u') {
        if (argc < 3) {
            fprintf(stderr, "mod: Missing argument to option '%s'\n" HELP_HELP, argv[1]);
            return 1;
        }

        int id = atoi(argv[2]);

        if (id == 0) {
            fprintf(stderr, "mod: Invalid id '%s'\n" HELP_HELP, argv[2]);
            return 1;
        }

        printf("mod: Unloading %d\n", id);

        syscall_process_set_scheduler(0);
        syscall_dily_unload(id);
        syscall_process_set_scheduler(1);
        return 0;
    }

    if (option == 'l') {
        if (argc < 4) {
            fprintf(stderr, "mod: Missing argument to option '%s'\n" HELP_HELP, argv[1]);
            return 1;
        }

        int id = atoi(argv[2]);

        if (id == 0) {
            fprintf(stderr, "mod: Invalid id '%s'\n" HELP_HELP, argv[2]);
            return 1;
        }

        // assemble new path
        char *new_path = assemble_path(pwd, argv[3]);

        if (!is_file(new_path)) {
            fprintf(stderr, "mod: '%s': File not found\n" HELP_HELP, new_path);
            return 1;
        }

        printf("mod: Loading %s as %d\n", new_path, id);

        syscall_dily_load(new_path, id);

        free(new_path);
        return 0;
    }

    if (option == 'r') {
        if (argc < 4) {
            fprintf(stderr, "mod: Missing argument to option '%s'\n" HELP_HELP, argv[1]);
            return 1;
        }

        int id = atoi(argv[2]);

        if (id == 0) {
            fprintf(stderr, "mod: Invalid id '%s'\n" HELP_HELP, argv[2]);
            return 1;
        }

        // assemble new path
        char *new_path = assemble_path(pwd, argv[3]);

        if (!is_file(new_path)) {
            fprintf(stderr, "mod: '%s': File not found\n" HELP_HELP, new_path);
            return 1;
        }

        printf("mod: Replacing %d with %s\n", id, new_path);

        syscall_process_set_scheduler(0);
        syscall_dily_unload(id);
        syscall_dily_load(new_path, id);
        syscall_process_set_scheduler(1);

        free(new_path);
        return 0;
    }

    printf("mod: Invalid option -- '%s'\n" HELP_HELP, argv[1]);
    return 1;
}
