/*****************************************************************************\
|   === wait.c : 2024 ===                                                     |
|                                                                             |
|    Implementation of sys/wait functions from libC                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <sys/wait.h>

#include <stdio.h>
#include <errno.h>

pid_t waitpid(pid_t pid, int *status, int options) {
    if (options & WUNTRACED || options & WCONTINUED) {
        fprintf(stderr, "waitpid: WUNTRACED and WCONTINUED not supported\n");
        errno = EINVAL;
        return -1;
    }

    if (pid == 0 || pid < -1) {
        fprintf(stderr, "waitpid: pid %d not supported\n", pid);
        errno = EINVAL;
        return -1;
    }

    uint8_t retcode;

    pid = syscall_process_wait(pid, &retcode, !(options & WNOHANG));

    if (pid < 0) {
        errno = ECHILD;
        return -1;
    }

    if (pid == 0 && options & WNOHANG) {
        return 0;
    }

    if (status)
        *status = retcode << 8;

    return pid;
}

pid_t wait(int *status) {
    return waitpid(-1, status, 0);
}
