/*****************************************************************************\
|   === md5sum.c : 2025 ===                                                   |
|                                                                             |
|    Unix command implementation - calculate MD5 checksums         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/carp.h>
#include <profan/md5.h>
#include <stdio.h>

int do_sum(const char *path, FILE *file) {
    uint8_t digest[16];

    if (file == NULL || md5_stream(file, digest)) {
        fprintf(stderr, "md5sum: %s: %m\n", path);
        if (file)
            fclose(file);
        return 1;
    }

    for (int i = 0; i < 16; i++)
        printf("%02x", digest[i]);
    printf("  %s\n", path);

    fclose(file);
    return 0;
}

int main(int argc, char **argv) {
    carp_init("[file1] [file2] ...", CARP_FNOMAX);

    if (carp_parse(argc, argv))
        return 1;

    const char *path;

    int error = 0;

    if (carp_file_count() == 0)
        return do_sum("-", stdin);

    while ((path = carp_file_next()))
        error |= do_sum(path, fopen(path, "rb"));

    return error;
}
