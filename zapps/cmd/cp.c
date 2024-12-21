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

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>

typedef struct {
    char *src;
    char *dst;
    int max_size;
    uint32_t block_size;
    int time_it;
} cp_args_t;

#define CP_USAGE "Usage: cp [options] <src> <dst>\n"

void cp_help(void) {
    fputs(CP_USAGE, stdout);
    puts("Options:\n"
        "  -b  Set the block size\n"
        "  -h  Show this help\n"
        "  -s  Stop after copying SIZE bytes\n"
        "  -t  Time the operation"
    );
}

int64_t atoi_unit(char *str) {
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

char *dest_check_dir(char *dst, char *src) {
    // Check if the destination is a directory
    // If it is, append the source file name to it

    uint32_t sid = profan_resolve_path(dst);

    if (!fu_is_dir(sid))
        return dst;

    char *fullpath, *src_name;

    profan_sep_path(src, NULL, &src_name);
    fullpath = profan_join_path(dst, src_name);
    free(src_name);

    return fullpath;
}

cp_args_t *cp_parse_args(int argc, char **argv) {
    cp_args_t *args = malloc(sizeof(cp_args_t));
    args->block_size = 4096;
    args->max_size = -1;
    args->time_it = 0;

    int i = 1;
    int64_t n;
    for (; i < argc && argv[i][0] == '-'; i++) {
        switch (argv[i][1]) {
            case 'b':
                n = atoi_unit(argv[++i]);
                if (n > 0 && n < INT_MAX)
                    args->block_size = n;
                else {
                    fprintf(stderr, "cp: %s: invalid block size\n", argv[i]);
                    exit(1);
                }
                break;
            case 's':
                n = atoi_unit(argv[++i]);
                if (n > 0 && n < INT_MAX)
                    args->max_size = n;
                else {
                    fprintf(stderr, "cp: %s: invalid size\n", argv[i]);
                    exit(1);
                }
                break;
            case 't':
                args->time_it = 1;
                break;
            case 'h':
                cp_help();
                exit(0);
            case '-':
                fprintf(stderr, "cp: unrecognized option '%s'\n", argv[i]);
                fputs(CP_USAGE, stderr);
                exit(1);
            default:
                fprintf(stderr, "cp: invalid option -- '%c'\n", argv[i][1]);
                fputs(CP_USAGE, stderr);
                exit(1);
        }
    }

    if (argc - i != 2) {
        fputs(CP_USAGE, stderr);
        exit(1);
    }

    args->src = argv[i];
    args->dst = argv[i + 1];

    args->dst = dest_check_dir(args->dst, args->src);

    return args;
}

int main(int argc, char **argv) {
    cp_args_t *args = cp_parse_args(argc, argv);

    int src_fd = open(args->src, O_RDONLY);
    if (src_fd == -1) {
        fprintf(stderr, "cp: %s: Unreadable file\n", args->src);
        exit(1);
    }

    int dst_fd = open(args->dst, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dst_fd == -1) {
        fprintf(stderr, "cp: %s: Failed to open file\n", args->dst);
        exit(1);
    }

    char *buf = malloc(args->block_size);
    uint32_t debut, to_read, total = 0;

    debut = args->time_it ? syscall_timer_get_ms() : 0;

    do {
        to_read = args->block_size;
        if (args->max_size != -1 && total + to_read > (uint32_t) args->max_size)
            to_read = args->max_size - total;

        int n = read(src_fd, buf, to_read);

        if (n == 0)
            break;

        if (n == -1) {
            fprintf(stderr, "cp: %s: Read error\n", args->src);
            exit(1);
        }

        if (write(dst_fd, buf, n) != n) {
            fprintf(stderr, "cp: %s: Write error\n", args->dst);
            exit(1);
        }

        total += n;
    } while (to_read == args->block_size);

    if (args->time_it) {
        int fin = syscall_timer_get_ms();
        fprintf(stderr, "cp: %d bytes copied in %d ms with %d bytes block size\n",
                total, fin - debut, args->block_size);
    }

    close(src_fd);
    close(dst_fd);
    return 0;
}
