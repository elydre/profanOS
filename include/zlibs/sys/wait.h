/*****************************************************************************\
|   === wait.h : 2024 ===                                                     |
|                                                                             |
|    Implementation of the sys/wait.h header file from libC        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/types.h>

#define _WSTOPPED       0x7f     // _WSTATUS if process is stopped
#define _WCONTINUED     0xffff   // process has continued
#define _WSTATUS(x)     ((x) & _WSTOPPED)

#define WIFSTOPPED(x)   (((x) & 0xff) == _WSTOPPED)
#define WSTOPSIG(x)     (int)(((unsigned)(x) >> 8) & 0xff)
#define WIFSIGNALED(x)  (_WSTATUS(x) != _WSTOPPED && _WSTATUS(x) != 0)
#define WTERMSIG(x)     (_WSTATUS(x))
#define WIFEXITED(x)    (_WSTATUS(x) == 0)
#define WEXITSTATUS(x)  (int)(((unsigned)(x) >> 8) & 0xff)
#define WIFCONTINUED(x) (((x) & _WCONTINUED) == _WCONTINUED)

#define WNOHANG     0x01    // don't hang in wait
#define WUNTRACED   0x02    // report stopped-by-signal processes
#define WCONTINUED  0x08    // report job control continued processes

pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);

#endif
