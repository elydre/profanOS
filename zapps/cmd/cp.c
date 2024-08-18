/*****************************************************************************\
|   === cp.c : 2024 ===                                                       |
|                                                                             |
|    Unix command implementation - clears the terminal screen      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/filesys.h>
#include <profan/syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int raise_and_free(char *msg, char *name, char *buf) {
    fprintf(stderr, "cp: Cannot copy '%s': %s\n", name, msg);
    free(buf);
    return 1;
}

int copy_elem(sid_t src_sid, char *src_path, char *dst_path) {
    if (!fu_is_file(src_sid)) {
        return raise_and_free("Not a file", src_path, NULL);
    }

    char *parent;
    char *name;

    fu_sep_path(dst_path, &parent, &name);

    sid_t parent_sid = fu_path_to_sid(ROOT_SID, parent);
    free(parent);
    free(name);

    if (IS_NULL_SID(parent_sid))
        return raise_and_free("No such file or directory", dst_path, NULL);

    sid_t new_sid = fu_path_to_sid(ROOT_SID, dst_path);

    if (IS_NULL_SID(new_sid)) {
        new_sid = fu_file_create(0, dst_path);
        if (IS_NULL_SID(new_sid))
            return raise_and_free("Failed to create file", dst_path, NULL);
    } else if (IS_SAME_SID(src_sid, new_sid)) {
        fprintf(stderr, "cp: '%s' and '%s' are the same file\n", src_path, dst_path);
        return 1;
    }

    if (IS_NULL_SID(new_sid))
        return raise_and_free("Failed to create file", dst_path, NULL);

    int size = fu_get_file_size(src_sid);
    if (size < 0)
        return raise_and_free("Failed to get file size", src_path, NULL);

    char *buf = malloc(size);
    if (fu_file_read(src_sid, buf, 0, size))
        return raise_and_free("Failed to read file", src_path, buf);

    if (syscall_fs_cnt_set_size(syscall_fs_get_main(), new_sid, size))
        return raise_and_free("Failed to set file size", dst_path, buf);

    if (fu_file_write(new_sid, buf, 0, size))
        return raise_and_free("Failed to write file", dst_path, buf);

    free(buf);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 3 && (argv[1][0] == '-' || argv[2][0] == '-')) {
        fputs("cp: Invalid argument\n", stderr);
        argc = 0;
    }

    if (argc != 3) {
        fputs("Usage: cp <src> <dst>\n", stderr);
        return 1;
    }

    char *pwd = getenv("PWD");
    if (pwd == NULL) pwd = "/";

    char *src_path = assemble_path(pwd, argv[1]);
    char *dst_path = assemble_path(pwd, argv[2]);

    sid_t src_sid = fu_path_to_sid(ROOT_SID, src_path);
    if (IS_NULL_SID(src_sid)) {
        fprintf(stderr, "cp: Cannot copy '%s': No such file or directory\n", src_path);
        free(src_path);
        free(dst_path);
        return 1;
    }

    if (fu_is_dir(src_sid)) {
        fprintf(stderr, "cp: Cannot copy '%s': Is a directory\n", src_path);
        free(src_path);
        free(dst_path);
        return 1;
    }

    sid_t dst_sid = fu_path_to_sid(ROOT_SID, dst_path);
    if (!IS_NULL_SID(dst_sid) && fu_is_dir(dst_sid)) {
        char *name;
        fu_sep_path(src_path, NULL, &name);
        char *new_dst_path = assemble_path(dst_path, name);
        free(dst_path);
        dst_path = new_dst_path;
        free(name);
    }

    int ret = copy_elem(src_sid, src_path, dst_path);

    free(src_path);
    free(dst_path);
    return ret;
}
