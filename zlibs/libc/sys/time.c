/*****************************************************************************\
|   === time.c : 2024 ===                                                     |
|                                                                             |
|    Implementation of sys/time functions from libC                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan.h>

#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

int getitimer(int which, struct itimerval *value) {
    return (PROFAN_FNI, -1);
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
    if (tz) {
        fputs("libc: gettimeofday: timezone is not supported\n", stderr);
        errno = ENOSYS;
        return -1;
    }

    if (tv == NULL) {
        errno = EFAULT;
        return -1;
    }

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        return -1;
    }

    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec / 1000;

    return 0;
}

int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value) {
    return (PROFAN_FNI, -1);
}

int utimes(const char *filename, const struct timeval *times) {
    return (PROFAN_FNI, -1);
}
