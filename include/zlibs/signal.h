/*****************************************************************************\
|   === signal.h : 2025 ===                                                   |
|                                                                             |
|    Implementation of the signal.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _SIGNAL_H
#define _SIGNAL_H

#define SIG_IGN ((void (*)(int)) 0)
#define SIG_INT ((void (*)(int)) 0)
#define SIG_ERR ((void (*)(int)) 0)
#define SIG_DFL ((void (*)(int)) 0)

#define SIGINT 2
#define SIGFPE 8
#define SIGABRT 6
#define SIGSEGV 11

typedef int sig_atomic_t;

void (*signal(int sig, void (*func)(int)))(int);
int kill(int pid, int sig);
int raise(int sig);

#endif
