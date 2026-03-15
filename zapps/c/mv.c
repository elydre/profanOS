/*****************************************************************************\
|   === mv.c : 2024 ===                                                       |
|                                                                             |
|    Unix command implementation - move file quickly               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <modules/filesys.h>
#include <profan/carp.h>
#include <profan.h>

#include <sys/stat.h> // mkdir
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

int DONT_RENAME = 0;

int move_element(const char *src, const char *dst, int verbose_indent);

int rm_file_perror(const char *path) {
    if (unlink(path) >= 0)
        return 0;
    perror("mv: unlink");
    return 1;
}

char *dest_check_dir(const char *dst, const char *src, uint32_t src_sid, int force) {
    // Check if the destination is a directory
    // If it is, append the source file name to it

    uint32_t sid = profan_path_resolve(dst);

    if (IS_SID_NULL(sid)) { // destination does not exist
        if (dst[0] != '/')
            return profan_path_join(profan_wd_path(), dst);
        else
            return strdup(dst);
    }

    if (IS_SAME_SID(src_sid, sid)) {
        fprintf(stderr, "mv: '%s' and '%s' are the same file\n", src, dst);
        return NULL;
    }

    if (!fu_is_dir(sid)) { // destination is an existing file
        if (fu_is_dir(src_sid))
            fprintf(stderr, "mv: Cannot overwrite non-directory '%s' with directory '%s'\n", dst, src);
        else if (!force)
            fprintf(stderr, "mv: '%s': File exists, try mv -f\n", dst);
        else if (dst[0] != '/')
            return rm_file_perror(dst) ? NULL : profan_path_join(profan_wd_path(), dst);
        else
            return rm_file_perror(dst) ? NULL : strdup(dst);
        return NULL;
    }

    char *fullpath, *src_name;

    profan_path_sep(src, NULL, &src_name);

    fullpath = malloc(strlen(profan_wd_path()) + strlen(dst) + strlen(src_name) + 3);
    if (dst[0] == '/')
        sprintf(fullpath, "%s/%s", dst, src_name);
    else
        sprintf(fullpath, "%s/%s/%s", profan_wd_path(), dst, src_name);
    profan_path_simplify(fullpath);

    free(src_name);
    return fullpath;
}

int is_a_subdir(uint32_t tofind, char *path) {
    // Check if the destination is a subdirectory of the source
    // by comparing the inodes of the source and destination directories

    uint32_t dst_sid = profan_path_resolve(path);
    if (IS_SAME_SID(tofind, dst_sid))
        return 1;

    if (strcmp(path, "/") == 0)
        return 0;

    char *parent = profan_path_join(path, "..");
    profan_path_simplify(parent);

    int res = is_a_subdir(tofind, parent);
    free(parent);
    return res;
}

int move_dir(uint32_t base, const char *src_path, const char *dest_path, int verbose_indent) {
    if (verbose_indent >= 0) {
        for (int i = 0; i < verbose_indent; i++)
            printf("  ");
        printf("C %s -> %s\n", src_path, dest_path);
    }

    uint32_t *sids;
    char **names;

    if (!IS_SID_NULL(profan_path_resolve(dest_path))) {
        fprintf(stderr, "mv: '%s': Destination already exists\n", dest_path);
        return 1;
    }

    if (mkdir(dest_path, 0755)) {
        fprintf(stderr, "mv: Failed to create directory '%s'\n", dest_path);
        return 1;
    }

    // we can't use fu_dir_get_elm in the loop because the elements will
    // be removed from the source directory during the listing...

    int count = fu_dir_get_content(base, &sids, &names);
    if (count < 0) {
        fprintf(stderr, "mv: Failed to read directory '%s'\n", src_path);
        return 1;
    }

    int ret_code = 0;

    for (int i = 0; i < count; i++) {
        char *name = names[i];
        uint32_t sid = sids[i];

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        char *d = profan_path_join(dest_path, name);
        char *p = profan_path_join(src_path, name);

        if (fu_is_dir(sid)) {
            if (move_dir(sid, p, d, verbose_indent == -1 ? -1 : verbose_indent + 1))
                ret_code = 1;
        } else {
            if (move_element(p, d, verbose_indent == -1 ? -1 : verbose_indent + 1)) {
                fprintf(stderr, "mv: Failed to move file '%s' to '%s'\n", p, d);
                ret_code = 1;
            }
        }

        free(p);
        free(d);

        if (ret_code)
            break;
    }

    if (unlink(src_path) < 0) {
        fprintf(stderr, "mv: Failed to remove directory '%s'\n", src_path);
        ret_code = 1;
    }

    for (int i = 0; i < count; i++)
        profan_kfree(names[i]);
    profan_kfree(names);
    profan_kfree(sids);

    return ret_code;
}

int move_element(const char *src, const char *dst, int verbose_indent) {
    if (!DONT_RENAME) {
        if (rename(src, dst) == 0) {
            if (verbose_indent >= 0) {
                for (int i = 0; i < verbose_indent; i++)
                    printf("  ");
                printf("R %s -> %s\n", src, dst);
            }
            return 0;
        }
        if (errno != EXDEV) {
            fprintf(stderr, "mv: Failed to move '%s' to '%s'\n", src, dst);
            return 1;
        }
    }

    uint32_t src_sid = profan_path_resolve(src);
    if (fu_is_dir(src_sid)) {
        return move_dir(src_sid, src, dst, verbose_indent);
    }

    if (!fu_is_file(src_sid)) {
        fprintf(stderr, "mv: '%s': Not a regular file\n", src);
        return 1;
    }

    // copy+delete fallback
    if (verbose_indent >= 0) {
        for (int i = 0; i < verbose_indent; i++)
            printf("  ");
        printf("C %s -> %s\n", src, dst);
    }

    FILE *in = fopen(src, "rb");
    if (!in) {
        fprintf(stderr, "mv: Failed to open source file '%s'\n", src);
        return 1;
    }

    FILE *out = fopen(dst, "wb");
    if (!out) {
        fprintf(stderr, "mv: Failed to open destination file '%s'\n", dst);
        fclose(in);
        return 1;
    }

    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, bytes, out) != bytes) {
            fprintf(stderr, "mv: Failed to write to destination file '%s'\n", dst);
            fclose(in);
            fclose(out);
            return 1;
        }
    }

    if (ferror(in)) {
        fprintf(stderr, "mv: Failed to read from source file '%s'\n", src);
        fclose(in);
        fclose(out);
        return 1;
    }

    fclose(in);
    fclose(out);

    if (unlink(src) < 0) {
        fprintf(stderr, "mv: Failed to remove source file '%s'\n", src);
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) {
    carp_init("[options] <source> <destination>", 2 | CARP_FMIN(2));

    carp_register('c', CARP_STANDARD, "force copy+delete instead of rename");
    carp_register('f', CARP_STANDARD, "overwrite existing files");
    carp_register('v', CARP_STANDARD, "explain what is being done");

    if (carp_parse(argc, argv))
        return 1;

    DONT_RENAME = carp_isset('c');

    const char *src = carp_file_next();
    const char *dst = carp_file_next();

    uint32_t src_sid = profan_path_resolve(src);
    if (IS_SID_NULL(src_sid)) {
        fprintf(stderr, "mv: '%s': No such file or directory\n", src);
        return 1;
    }

    char *full_dst = dest_check_dir(dst, src, src_sid, carp_isset('f'));
    if (full_dst == NULL)
        return 1;

    if (fu_is_dir(src_sid) && is_a_subdir(src_sid, full_dst)) {
        fprintf(stderr, "mv: '%s' is a subdirectory of '%s'\n", full_dst, src);
        free(full_dst);
        return 1;
    }

    if (move_element(src, full_dst, carp_isset('v') ? 0 : -1)) {
        fprintf(stderr, "mv: Failed to move '%s' to '%s'\n", src, full_dst);
        free(full_dst);
        return 1;
    }

    free(full_dst);
    return 0;
}
