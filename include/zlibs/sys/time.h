/*****************************************************************************\
|   === time.h : 2024 ===                                                     |
|                                                                             |
|    Implementation of the sys/time.h header file from libC        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <sys/types.h>

struct timeval {
    time_t      tv_sec;  // seconds
    suseconds_t tv_usec; // and microseconds
};

struct itimerval {
    struct timeval it_interval; // timer interval
    struct timeval it_value;    // current value
};

struct timezone {
    int tz_minuteswest; // minutes west of Greenwich
    int tz_dsttime;     // type of dst correction
};

#define ITIMER_REAL     0
#define ITIMER_VIRTUAL  1
#define ITIMER_PROF     2

int getitimer(int which, struct itimerval *value);
int gettimeofday(struct timeval *tv, struct timezone *tz);
int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
int utimes(const char *filename, const struct timeval *times);

#endif
