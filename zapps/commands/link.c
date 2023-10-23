#include <filesys.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int print_help(int full) {
    puts("Usage: link <flag> [options]");

    if (!full) {
        puts("try 'link -h' for more information");
        return 1;
    }

    puts(
        "Options:\n"
        "  -a <link> <path> <pid> Add a path to the link\n"
        "  -c <link>              Create a link\n"
        "  -r <link> <pid>        Remove a path from the link\n"
        "  -g <link> <pid>        Get the path of a pid\n"
        "  -l <link>              List all paths\n"
        "  -h                     Print this help"
    );

    return 0;
}

#define ACTION_ADD      1
#define ACTION_CREATE   2
#define ACTION_REMOVE   3
#define ACTION_GET      4
#define ACTION_LIST     5
#define ACTION_HELP     6
#define ACTION_ERROR    7

typedef struct {
    int action;
    char *link;
    char *path;
    int pid;
} link_args_t;

link_args_t *parse_args(int argc, char **argv) {
    link_args_t *args = malloc(sizeof(link_args_t));
    args->action = ACTION_ERROR;
    args->link = NULL;
    args->path = NULL;
    args->pid = -1;

    if (argc < 2) {
        args->action = ACTION_ERROR;
        return args;
    }

    if (argv[1][0] != '-') {
        args->action = ACTION_ERROR;
        return args;
    }

    switch (argv[1][1]) {
        case 'a':
            if (argc != 5) {
                args->action = ACTION_ERROR;
                return args;
            }

            args->action = ACTION_ADD;
            args->link = argv[2];
            args->path = argv[3];
            args->pid = atoi(argv[4]);
            break;
        case 'c':
            if (argc != 3) {
                args->action = ACTION_ERROR;
                return args;
            }

            args->action = ACTION_CREATE;
            args->link = argv[2];
            break;
        case 'r':
            if (argc != 4) {
                args->action = ACTION_ERROR;
                return args;
            }

            args->action = ACTION_REMOVE;
            args->link = argv[2];
            args->pid = atoi(argv[3]);
            break;
        case 'g':
            if (argc != 4) {
                args->action = ACTION_ERROR;
                return args;
            }

            args->action = ACTION_GET;
            args->link = argv[2];
            args->pid = atoi(argv[3]);
            break;
        case 'l':
            if (argc != 3) {
                args->action = ACTION_ERROR;
                return args;
            }

            args->action = ACTION_LIST;
            args->link = argv[2];
            break;
        case 'h':
            args->action = ACTION_HELP;
            break;
        default:
            printf("Unknown option '%s'\n", argv[1]);
            args->action = ACTION_ERROR;
            break;
    }

    return args;
}

int main(int argc, char **argv) {
    link_args_t *args = parse_args(argc, argv);
    int ret = 0;

    if (args->action == ACTION_ERROR) {
        print_help(0);
        free(args);
        return 1;
    }

    if (args->action == ACTION_HELP) {
        print_help(1);
        free(args);
        return 0;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *path = malloc(strlen(pwd) + strlen(args->link) + 2);
    assemble_path(pwd, args->link, path);
    args->link = path;

    sid_t link_sid = fu_path_to_sid(ROOT_SID, args->link);

    if (IS_NULL_SID(link_sid)) {
        if (args->action != ACTION_CREATE) {
            printf("link '%s' does not exist\n", args->link);
            free(args->link);
            free(args);
            return 1;
        }
        ret = IS_NULL_SID(fu_link_create(0, args->link));
        free(args->link);
        free(args);
        return ret;
    }

    if (!fu_is_link(link_sid)) {
        printf("'%s' is not a link\n", args->link);
        free(args->link);
        free(args);
        return 1;
    }

    if (args->action == ACTION_CREATE) {
        printf("link '%s' already exists\n", args->link);
        free(args->link);
        free(args);
        return 1;
    }

    if (args->action == ACTION_ADD) {
        ret = fu_link_add_path(link_sid, args->pid, args->path);
        free(args->link);
        free(args);
        return ret;
    }

    if (args->action == ACTION_REMOVE) {
        ret = fu_link_remove_path(link_sid, args->pid);
        free(args->link);
        free(args);
        return ret;
    }

    if (args->action == ACTION_GET) {
        char *path = fu_link_get_path(link_sid, args->pid);

        if (path == NULL) {
            printf("pid %d not found in link '%s'\n", args->pid, args->link);
            free(args->link);
            free(args);
            return 1;
        }

        printf("%s\n", path);
        free(args->link);
        free(path);
        free(args);
        return 0;
    }

    if (args->action == ACTION_LIST) {
        char **paths;
        int *pids;
        int count = fu_link_get_all(link_sid, &pids, &paths);

        if (count == 0) {
            printf("link '%s' is empty\n", args->link);
            free(args->link);
            free(args);
            return 0;
        }

        for (int i = 0; i < count; i++) {
            printf("PID %d: %s\n", pids[i], paths[i]);
            free(paths[i]);
        }

        free(args->link);
        free(paths);
        free(pids);
        free(args);
        return 0;
    }

    puts("Internal error");
    free(args->link);
    free(args);
    return 1;
}
