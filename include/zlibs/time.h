/*****************************************************************************\
|   === time.h : 2024 ===                                                     |
|                                                                             |
|    Implementation of the time.h header file from libC            .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _TIME_H
#define _TIME_H

#include <profan/types.h> // sigevent_t
#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>

#define CLK_TCK 100         // Number of clock ticks per second returned by the times() function
#define CLOCKS_PER_SEC 1000 // Used to convert the value returned by the clock() function into seconds
#define CLOCK_REALTIME 0    // The identifier of the systemwide realtime clock
#define TIMER_ABSTIME 1     // Flag indicating time is absolute

#define CLOCK_MONOTONIC 1   // The identifier of the systemwide monotonic clock

struct tm {
    int    tm_sec;   // seconds [0,61]
    int    tm_min;   // minutes [0,59]
    int    tm_hour;  // hour [0,23]
    int    tm_mday;  // day of month [1,31]
    int    tm_mon;   // month of year [0,11]
    int    tm_year;  // years since 1900
    int    tm_wday;  // day of week [0,6] (Sunday = 0)
    int    tm_yday;  // day of year [0,365]
    int    tm_isdst; // daylight savings flag
};

struct timespec {
    time_t  tv_sec;  // seconds
    long    tv_nsec; // nanoseconds
};

struct itimerspec {
    struct timespec it_interval; // timer period
    struct timespec it_value;    // timer expiration
};

extern char    *tzname[2];
extern long int timezone;
extern int      daylight;

char      *asctime(const struct tm *timeptr);
clock_t    clock(void);
int        clock_getres(clockid_t clock_id, struct timespec *res);
int        clock_gettime(clockid_t clock_id, struct timespec *tp);
int        clock_settime(clockid_t clock_id, const struct timespec *tp);
char      *ctime(const time_t *timer);
double     difftime(time_t end_time, time_t start_time);
struct tm *getdate(const char *string);
struct tm *gmtime(const time_t *timer);
struct tm *gmtime_r(const time_t *timer, struct tm *buf);
struct tm *localtime(const time_t *timer);
struct tm *localtime_r(const time_t *timer, struct tm *buf);
time_t     mktime(struct tm *timeptr);
int        nanosleep(const struct timespec *req, struct timespec *rem);
size_t     strftime(char *str, size_t maxsize, const char *format, const struct tm *timeptr);
char      *strptime(const char *buf, const char *format, struct tm *timeptr);
time_t     time(time_t *tloc);
int        timer_create(clockid_t clock_id, struct sigevent *sevp, timer_t *timerid);
int        timer_delete(timer_t timerid);
int        timer_gettime(timer_t timerid, struct itimerspec *curr_value);
int        timer_getoverrun(timer_t timerid);
int        timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value,
                    struct itimerspec *old_value);
void       tzset(void);

#endif
