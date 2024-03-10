#include <i_memreg.h>
#include <stdlib.h>
#include <unistd.h>
#include <filesys.h>
#include <profan.h>
#include <stdio.h>
#include <string.h>

char *get_path(char *file) {
    char *path = getenv("PATH");
    
    if (file[0] == '/') {
        return strdup(file);
    }

    if (file[0] == '.' && file[1] == '/') {
        return assemble_path(getenv("PWD"), file + 2);
    }

    if (path == NULL) {
        return strdup(file);
    }

    char *tmp = malloc(strlen(file) + 5);
    strcpy(tmp, file);
    strcat(tmp, ".bin");


    for (char *p = path; *p != '\0'; p++) {
        if (*p == ':') {
            char *dir = malloc(p - path + 1);
            memcpy(dir, path, p - path);
            dir[p - path] = '\0';

            char *full = assemble_path(dir, tmp);
            free(dir);
            if (fu_is_file(fu_path_to_sid(ROOT_SID, full))) {
                free(tmp);
                return full;
            }
            free(full);
            path = p + 1;
        }
    }
    free(tmp);

    return strdup(file);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <command> [args]\n", argv[0]);
        return 1;
    }

    int already_activate = manage_malloc_debug(1);

    int pid;

    char *path = get_path(argv[1]);
    sid_t sid = fu_path_to_sid(ROOT_SID, path);
    if (IS_NULL_SID(sid) || !fu_is_file(sid)) {
        printf("Error: %s not found\n", path);
        free(path);
        return 1;
    }

    run_ifexist_full((runtime_args_t){path, sid,
        argc - 1, argv + 1, 0, 1}, &pid);

    free(path);

    int ret_code = profan_wait_pid(pid);

    memreg_dump(pid, isatty(1));

    if (!already_activate) {
        manage_malloc_debug(0);
    }

    return ret_code;    
}
