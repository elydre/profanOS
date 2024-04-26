/*****************************************************************************\
|   === fmclean.c : 2024 ===                                                  |
|                                                                             |
|    Command to clean the fmopen lib cache                         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/filesys.h>

int main(void) {
    fm_clean();
    return 0;
}
