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
    profan_nimpl("dlclose");
    return -1;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
    if (tv) {
        tv->tv_sec = time(NULL);
        tv->tv_usec = (syscall_timer_get_ms() % 1000) * 1000;
    }
    if (tz) {
        fprintf(stderr, "gettimeofday: timezone is not supported\n");
        errno = ENOSYS;
        return -1;
    }
    return 0;
}

int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value) {
    profan_nimpl("dlclose");
    return -1;
}

int utimes(const char *filename, const struct timeval *times) {
    profan_nimpl("dlclose");
    return -1;
}
