#include <modules/filesys.h>
#include <profan.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <disk> <mount_point>\n", argv[0]);
        return 1;
    }

    sid_t disk = profan_path_resolve(argv[1]);

    if (!fu_is_afft(disk)) {
        fprintf(stderr, "Error: '%s' is not a valid afft\n", argv[1]);
        return 1;
    }

    int afft = fu_afft_get_id(disk);

    if (afft < 0) {
        fprintf(stderr, "Error: Failed to get afft ID for disk '%s'\n", argv[1]);
        return 1;
    }

    char *fullpath = profan_path_join(profan_wd_path(), argv[2]);
    char *parent, *cnt;

    profan_path_sep(fullpath, &parent, &cnt);

    sid_t mount_point = profan_path_resolve(parent);

    if (!fu_is_dir(mount_point)) {
        fprintf(stderr, "Error: Invalid mount point '%s'\n", parent);
        return 1;
    }

    if (fu_mount_afft(afft, mount_point, cnt)) {
        fprintf(stderr, "Error: Failed to mount '%s' at '%s'\n", argv[1], argv[2]);
        return 1;
    }

    free(fullpath);
    return 0;
}
