/*****************************************************************************\
|   === upper.c : 2024 ===                                                    |
|                                                                             |
|    Unix command implementation - convert stdin to uppercase      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <unistd.h>

int main(void) {
    char c;
    while (read(0, &c, 1) > 0) {
        if (c >= 'a' && c <= 'z') {
            c -= 32;
        }
        write(1, &c, 1);
    }
    return 0;
}
