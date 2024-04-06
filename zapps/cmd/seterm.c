#include <profan/filesys.h>
#include <profan.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int is_valid_file(char *path) {
    sid_t sid = fu_path_to_sid(ROOT_SID, path);
    if (IS_NULL_SID(sid)) {
        fprintf(stderr, "seterm: %s: No such file or directory\n", path);
        return 0;
    }
    if (!(fu_is_file(sid) || fu_is_fctf(sid))) {
        fprintf(stderr, "seterm: %s: Not a file or a fctf\n", path);
        return 0;
    }
    return 1;
}

char *process_path(char *path) {
    char *cwd = getenv("PWD");
    if (!cwd) cwd = "/";
    return assemble_path(cwd, path);
}

int main(int argc, char **argv) {
    if (argc != 2 || argv[1][0] == '-') {
        fprintf(stderr, "Usage: seterm <file>\n", argv[0]);
        return 1;
    }

    char *path = process_path(argv[1]);

    if (!is_valid_file(argv[1])) {
        free(path);
        return 1;
    }

    fm_reopen(3, path);
    fm_reopen(4, path);
    fm_reopen(5, path);

    int ppid = getppid();

    fm_reopen(fm_resol012(0, ppid), path);
    fm_reopen(fm_resol012(1, ppid), path);
    fm_reopen(fm_resol012(2, ppid), path);

    setenv("TERM", path, 1);

    free(path);
    return 0;
}
