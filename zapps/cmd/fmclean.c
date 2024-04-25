/****** This file is part of profanOS **************************\
|   == fmclean.c ==                                  .pi0iq.    |
|                                                   d"  . `'b   |
|   Command to clean the fmopen lib cache           q. /|\ .p   |
|                                                    '// \\'    |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <profan/filesys.h>

int main(void) {
    fm_clean();
    return 0;
}
