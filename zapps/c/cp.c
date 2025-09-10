/*****************************************************************************\
|   === cp.c : 2024 ===                                                       |
|                                                                             |
|    Unix style file copy with features from dd                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/carp.h>
#include <profan.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>

int64_t atoi_unit(const char *str) {
    // Parse a number with a unit sucpix
    // k, m, g / kb, mb, gb / ko, mo, go
    // no case sensitive, return -1 on error

    int64_t n;

    if (*str < '0' || *str > '9')
        return -1;

    for (n = 0; *str >= '0' && *str <= '9'; str++)
        n = n * 10 + *str - '0';

    if (*str == '\0')
        return n;

    if (strlen(str) > 2)
        return -1;

    switch (str[0]) {
        case 'k':
        case 'K':
            n *= 1024;
            break;
        case 'm':
        case 'M':
            n *= 1024 * 1024;
            break;
        case 'g':
        case 'G':
            n *= 1024 * 1024 * 1024;
            break;
        default:
            return -1;
    }

    if (!*++str)
        return n;

    for (int j = 0; j < 4; j++)
        if (*str == "BbOo"[j])
            return n;

    return -1;
}

const char *dest_check_dir(const char *dst, const char *src) {
    // Check if the destination is a directory
    // If it is, append the source file name to it

    uint32_t sid = profan_path_resolve(dst);

    if (!fu_is_dir(sid))
        return dst;

    char *fullpath, *src_name;

    profan_path_sep(src, NULL, &src_name);
    fullpath = profan_path_join(dst, src_name);
    free(src_name);

    return fullpath;
}

typedef struct {
    const char *src;
    const char *dst;
    uint32_t block_size;
    int max_size;
    int time_it;
} cp_args_t;

cp_args_t *cp_parse_args(int argc, char **argv) {
    carp_init("[options] <src> <dst>", CARP_FMIN(2));

    carp_register('b', CARP_NEXT_STR, "set the block size");
    carp_register('s', CARP_NEXT_STR, "set copy size limit");
    carp_register('t', CARP_STANDARD, "time the operation");

    if (carp_parse(argc, argv))
        exit(1);

    cp_args_t *args = malloc(sizeof(cp_args_t));

    if (carp_isset('b')) {
        int n = atoi_unit(carp_get_str('b'));
        if (n > 0 && n < INT_MAX)
            args->block_size = n;
        else {
            fprintf(stderr, "cp: %s: invalid block size\n", carp_get_str('b'));
            exit(1);
        }
    } else {
        args->block_size = 4096;
    }

    if (carp_isset('s')) {
        int n = atoi_unit(carp_get_str('s'));
        if (n > 0)
            args->max_size = n;
        else {
            fprintf(stderr, "cp: %s: invalid size\n", carp_get_str('s'));
            exit(1);
        }
    } else {
        args->max_size = -1;
    }

    args->time_it = carp_isset('t');

    args->src = carp_file_next();
    args->dst = dest_check_dir(carp_file_next(), args->src);

    return args;
}

int main(int argc, char **argv) {
    cp_args_t *args = cp_parse_args(argc, argv);

    int src_fd = open(args->src, O_RDONLY | O_NODIR);
    if (src_fd == -1) {
        fprintf(stderr, "cp: %s: %m\n", args->src);
        return 1;
    }

    int dst_fd = open(args->dst, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dst_fd == -1) {
        fprintf(stderr, "cp: %s: %m\n", args->dst);
        return 1;
    }

    char *buf = malloc(args->block_size);
    uint32_t debut, to_read, total = 0;

    debut = args->time_it ? syscall_ms_get() : 0;

    do {
        to_read = args->block_size;
        if (args->max_size != -1 && total + to_read > (uint32_t) args->max_size)
            to_read = args->max_size - total;

        int n = read(src_fd, buf, to_read);

        if (n == 0)
            break;

        if (n == -1) {
            fprintf(stderr, "cp: %s: %m\n", args->src);
            return 1;
        }

        if (write(dst_fd, buf, n) != n) {
            fprintf(stderr, "cp: %s: %m\n", args->dst);
            return 1;
        }

        total += n;
    } while (to_read == args->block_size);

    if (args->time_it) {
        int fin = syscall_ms_get();
        fprintf(stderr, "cp: %d bytes copied in %d ms with %d bytes block size\n",
                total, fin - debut, args->block_size);
    }

    close(src_fd);
    close(dst_fd);
    return 0;
}
