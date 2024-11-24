/*****************************************************************************\
|   === wait.h : 2024 ===                                                     |
|                                                                             |
|    Implementation of sys/wait function from libC                 .pi0iq.    |
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
    if (options != 0) {
        fprintf(stderr, "waitpid: options not supported\n");
        errno = EINVAL;
        return -1;
    }

    if (pid == 0 || pid < -1) {
        fprintf(stderr, "waitpid: pid not supported\n");
        errno = EINVAL;
        return -1;
    }

    pid = syscall_process_wait(pid);

    if (pid < 0) {
        errno = -pid;
        return -1;
    }

    if (status)
        *status = syscall_process_info(pid, PROCESS_INFO_EXIT_CODE) << 8;

    return pid;
}

pid_t wait(int *status) {
    return waitpid(-1, status, 0);
}
