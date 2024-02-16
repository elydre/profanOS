#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>


#define HELP_USAGE "Usage: lib <flag> [args]\n"
#define HELP_HELP "Try 'lib -h' for more information.\n"

int is_file(char *path) {
    sid_t sid = fu_path_to_sid(ROOT_SID, path);
    if (IS_NULL_SID(sid)) return 0;
    return fu_is_file(sid);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf(HELP_USAGE HELP_HELP);
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char flag = *(argv[1] + 1);
    if (flag == '-') {
        flag = *(argv[1] + 2);
    }

    if (flag == '\0') {
        printf("lib: invalid flag '%s'\n" HELP_HELP, argv[1]);
        return 1;
    }

    if (flag == 'h') {
        printf(HELP_USAGE);
        printf("Available flags:\n");
        printf("  -h              print this help message\n");
        printf("  -u <id>         unload a library\n");
        printf("  -l <id> <path>  load a library\n");
        printf("  -r <id> <path>  replace a library\n");
        return 0;
    }

    if (flag == 'u') {
        if (argc < 3) {
            printf("lib: missing argument to flag '%s'\n" HELP_HELP, argv[1]);
            return 1;
        }

        int id = atoi(argv[2]);

        if (id == 0) {
            printf("lib: invalid id '%s'\n" HELP_HELP, argv[2]);
            return 1;
        }

        printf("lib: unloading %d\n", id);

        c_process_set_sheduler(0);
        c_dily_unload(id);
        c_process_set_sheduler(1);
        return 0;
    }

    if (flag == 'l') {
        if (argc < 4) {
            printf("lib: missing argument to flag '%s'\n" HELP_HELP, argv[1]);
            return 1;
        }

        int id = atoi(argv[2]);

        if (id == 0) {
            printf("lib: invalid id '%s'\n" HELP_HELP, argv[2]);
            return 1;
        }

        // assemble new path
        char *new_path = assemble_path(pwd, argv[3]);

        if (!is_file(new_path)) {
            printf("lib: file '%s' does not exist\n" HELP_HELP, new_path);
            return 1;
        }

        printf("lib: loading %s as %d\n", new_path, id);

        c_dily_load(new_path, id);

        free(new_path);
        return 0;
    }

    if (flag == 'r') {
        if (argc < 4) {
            printf("lib: missing argument to flag '%s'\n" HELP_HELP, argv[1]);
            return 1;
        }

        int id = atoi(argv[2]);

        if (id == 0) {
            printf("lib: invalid id '%s'\n" HELP_HELP, argv[2]);
            return 1;
        }

        // assemble new path
        char *new_path = assemble_path(pwd, argv[3]);

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

    printf("lib: invalid flag '%s'\n" HELP_HELP, argv[1]);
    return 1;
}
