/*****************************************************************************\
|   === sc.c : 2024 ===                                                       |
|                                                                             |
|    Keyboard scancode viewer                                      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan.h>

#include <unistd.h>
#include <stdio.h>

int main(void) {
    int last_sc = 0;
    char min_val, max_val;

    printf("enter scancode, press ESC to exit\n");

    while ((last_sc = syscall_kb_get_scfh()) != 1) {
        if (last_sc == 0) {
            usleep(10000);
            continue;
        }

        min_val = profan_kb_get_char(last_sc, 0);
        max_val = profan_kb_get_char(last_sc, 1);

        printf("scancode: %d [%c %c]\n",
                last_sc,
                min_val ? min_val : '?',
                max_val ? max_val : '?'
        );
    }
    return 0;
}
