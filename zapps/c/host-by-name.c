/*****************************************************************************\
|   === host-by-name.c : 2026 ===                                             |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdio.h>
#include <netdb.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        return 1;
    }
    struct hostent *info = gethostbyname(argv[1]);
    if (!info) {
        fprintf(stderr, "%s: domain name not found\n", argv[0]);
        return 1;
    }
    printf("%d.%d.%d.%d\n",
        (uint8_t)info->h_addr_list[0][0],
        (uint8_t)info->h_addr_list[0][1],
        (uint8_t)info->h_addr_list[0][2],
        (uint8_t)info->h_addr_list[0][3]
    );
    return 0;
}
