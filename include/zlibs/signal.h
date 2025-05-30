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

#include <profan/minimal.h>

_BEGIN_C_FILE

#define _NSIG       64
#define _NSIG_BPW   32
#define _NSIG_WORDS (_NSIG / _NSIG_BPW)

#define SIGHUP      1
#define SIGINT      2
#define SIGQUIT     3
#define SIGILL      4
#define SIGTRAP     5
#define SIGABRT     6
#define SIGIOT      6
#define SIGBUS      7
#define SIGFPE      8
#define SIGKILL     9
#define SIGUSR1     10
#define SIGSEGV     11
#define SIGUSR2     12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15
#define SIGSTKFLT   16
#define SIGCHLD     17
#define SIGCONT     18
#define SIGSTOP     19
#define SIGTSTP     20
#define SIGTTIN     21
#define SIGTTOU     22
#define SIGURG      23
#define SIGXCPU     24
#define SIGXFSZ     25
#define SIGVTALRM   26
#define SIGPROF     27
#define SIGWINCH    28
#define SIGIO       29
#define SIGPOLL     29
#define SIGPWR      30
#define SIGSYS      31
#define SIGUNUSED   31

#define SIGRTMIN    32
#define SIGRTMAX    _NSIG

typedef int sig_atomic_t;

#define SIG_IGN ((void (*)(int)) 0)
#define SIG_INT ((void (*)(int)) 0)
#define SIG_ERR ((void (*)(int)) 0)
#define SIG_DFL ((void (*)(int)) 0)

void (*signal(int sig, void (*func)(int)))(int);
int kill(int pid, int sig);
int raise(int sig);

_END_C_FILE

#endif
