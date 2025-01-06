/*****************************************************************************\
|   === signal.c : 2025 ===                                                   |
|                                                                             |
|    Implementation of signal functions from libC                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <signal.h>
#include <profan.h>

void (*signal(int sig, void (*func)(int)))(int) {
    return SIG_ERR; // (PROFAN_FNI, SIG_ERR);
}

int kill(int pid, int sig) {
    return (PROFAN_FNI, -1);
}

int raise(int sig) {
    return (PROFAN_FNI, -1);
}
