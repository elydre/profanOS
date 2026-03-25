/*****************************************************************************\
|   === init.c : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "mlw_private.h"

eth_info_t *mlw_info = NULL;

int mlw_init(uint32_t flags) {
    if (flags & MLW_INIT_RAND)
        srand(time(NULL));
    mlw_info = malloc(sizeof(eth_info_t));
    if (!mlw_info)
        return 1;
    modeth_get_info(0, mlw_info);
    return 0;
}
