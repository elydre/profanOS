/*****************************************************************************\
|   === mod.c : 2024 ===                                                      |
|                                                                             |
|    Command to load and unload kernel modules                     .pi0iq.    |
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    carp_init("[option] <id>", CARP_FMIN(1));

    carp_register('u', CARP_STANDARD, "unload a library");
    carp_register('l', CARP_NEXT_STR, "load/replace a library");

    if (carp_parse(argc, argv))
        return 1;

    const char *strid = carp_file_next();

    int id = atoi(strid);

    if (id == 0) {
        fprintf(stderr, "%s: invalid id '%s'\n", argv[0], strid);
        return 1;
    }

    if (carp_isset('u')) {
        printf("mod: unloading %d...\n", id);

        syscall_mod_unload(id);
        return 0;
    }

    if (carp_isset('l')) {
        char *new_path = profan_path_join(profan_wd_path(), carp_get_str('l'));

        if (!fu_is_file(fu_path_to_sid(SID_ROOT, new_path))) {
            fprintf(stderr, "mod: '%s': file not found\n", new_path);
            return 1;
        }

        printf("mod: Loading %s as %d\n", new_path, id);

        int error_code = syscall_mod_load(new_path, id);
        free(new_path);

        if (error_code > 0)
            return 0;

        fprintf(stderr, "mod: ", error_code);
        switch (error_code) {
            case -1:
                fputs("file not found\n", stderr);
                break;
            case -2:
                fputs("invalid module id range\n", stderr);
                break;
            case -3:
                fputs("file is not a dynamic ELF\n", stderr);
                break;
            case -4:
                fputs("ELF relocation failed\n", stderr);
                break;
            case -5:
                fputs("failed to read functions\n", stderr);
                break;
            case -6:
                fputs("init function exited with error\n", stderr);
                break;
            default:
                fputs("unknown error\n", stderr);
                break;
        }
        return 1;
    }

    return 1;
}
