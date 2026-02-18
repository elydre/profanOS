/*****************************************************************************\
|   === test.c : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <stdio.h>

#define pscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

#define MOD_ID 42

#define supercool_function() ((int) pscall(MOD_ID, 1, 1 /* avoid warning */))
#define sumit(a, b) ((int) pscall(MOD_ID, 0, a, b))


int main(void) {
    int ret = syscall_mod_load("/lib/modules/multifile.pkm", MOD_ID);

    if (ret < 0) {
        printf("Failed to load module, error code: %d\n", ret);
        return -1;
    }

    int result = supercool_function();
    printf("supercool_function() returned: %d\n", result);

    int sum_result = sumit(5, 7);
    printf("sumit(5, 7) returned: %d\n", sum_result);

    ret = syscall_mod_unload(MOD_ID);

    if (ret) {
        printf("Failed to unload module, error code: %d\n", ret);
        return -1;
    }

    return 0;
}
