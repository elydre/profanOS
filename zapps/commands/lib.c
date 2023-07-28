#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <profan.h>


#define HELP_USAGE "Usage: lib <mode> [args]\n"
#define HELP_HELP "Try 'lib -h' for more information.\n"

#define is_file(path) (c_fs_does_path_exists(path) && c_fs_get_sector_type(c_fs_path_to_id(path)) == 2)


int main(int argc, char **argv) {
    // parse arguments

    if (argc < 3) {
        printf(HELP_USAGE HELP_HELP);

        return 1;
    }

    char mode = *(argv[2] + 1);

    if (mode == '-') {
        mode = *(argv[2] + 2);
    }

    if (mode == '\0') {
        printf("lib: invalid mode '%s'\n" HELP_HELP, argv[2]);
        return 1;
    }

    if (mode == 'h') {
        printf(HELP_USAGE);
        printf("Available modes:\n");
        printf("  -h              print this help message\n");
        printf("  -u <id>         unload a library\n");
        printf("  -l <id> <path>  load a library\n");
        printf("  -r <id> <path>  replace a library\n");
        return 0;
    }

    if (mode == 'u') {
        if (argc < 4) {
            printf("lib: missing argument to mode '%s'\n" HELP_HELP, argv[2]);
            return 1;
        }

        int id = atoi(argv[3]);

        if (id == 0) {
            printf("lib: invalid id '%s'\n" HELP_HELP, argv[3]);
            return 1;
        }

        printf("lib: unloading %d\n", id);

        c_process_set_sheduler(0);
        c_dily_unload(id);
        c_process_set_sheduler(1);
        return 0;
    }

    if (mode == 'l') {
        if (argc < 5) {
            printf("lib: missing argument to mode '%s'\n" HELP_HELP, argv[2]);
            return 1;
        }

        int id = atoi(argv[3]);

        if (id == 0) {
            printf("lib: invalid id '%s'\n" HELP_HELP, argv[3]);
            return 1;
        }

        // assemble new path
        char *new_path = malloc(strlen(argv[1]) + strlen(argv[4]) + 3);
        assemble_path(argv[1], argv[4], new_path);

        if (!is_file(new_path)) {
            printf("lib: file '%s' does not exist\n" HELP_HELP, new_path);
            return 1;
        }

        printf("lib: loading %s as %d\n", new_path, id);

        c_dily_load(new_path, id);

        free(new_path);
        return 0;
    }

    if (mode == 'r') {
        if (argc < 5) {
            printf("lib: missing argument to mode '%s'\n" HELP_HELP, argv[2]);
            return 1;
        }

        int id = atoi(argv[3]);

        if (id == 0) {
            printf("lib: invalid id '%s'\n" HELP_HELP, argv[3]);
            return 1;
        }

        // assemble new path
        char *new_path = malloc(strlen(argv[1]) + strlen(argv[4]) + 3);
        assemble_path(argv[1], argv[4], new_path);

        if (!is_file(new_path)) {
            printf("lib: file '%s' does not exist\n" HELP_HELP, new_path);
            return 1;
        }

        printf("lib: replacing %d with %s\n", id, new_path);

        c_process_set_sheduler(0);
        c_dily_unload(id);
        c_dily_load(new_path, id);
        c_process_set_sheduler(1);

        free(new_path);
        return 0;
    }

    printf("lib: invalid mode '%s'\n" HELP_HELP, argv[2]);
    return 1;
}
