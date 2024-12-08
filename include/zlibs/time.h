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

typedef struct tm {
    int    tm_sec;   // seconds [0,61]
    int    tm_min;   // minutes [0,59]
    int    tm_hour;  // hour [0,23]
    int    tm_mday;  // day of month [1,31]
    int    tm_mon;   // month of year [0,11]
    int    tm_year;  // years since 1900
    int    tm_wday;  // day of week [0,6] (Sunday = 0)
    int    tm_yday;  // day of year [0,365]
    int    tm_isdst; // daylight savings flag
} tm_t;

typedef struct timespec {
    time_t  tv_sec;  // seconds
    long    tv_nsec; // nanoseconds
} timespec_t;

typedef struct itimerspec {
    timespec_t it_interval; // timer period
    timespec_t it_value;    // timer expiration
} itimerspec_t;

char *asctime(const struct tm *a);
char *asctime_r(const struct tm *a, char *b);
clock_t clock(void);
int clock_getres(clockid_t a, struct timespec *n);
int clock_gettime(clockid_t a, struct timespec *b);
int clock_settime(clockid_t a, const struct timespec *b);
char *ctime(const time_t *a);
char *ctime_r(const time_t *a, char *b);
double difftime(time_t a, time_t b);
struct tm *getdate(const char *a);
struct tm *gmtime(const time_t *a);
struct tm *gmtime_r(const time_t *a, struct tm *b);
struct tm *localtime(const time_t *a);
struct tm *localtime_r(const time_t *a, struct tm *b);
time_t mktime(struct tm *time);
int nanosleep(const struct timespec *a, struct timespec *b);
size_t strftime(char *a, size_t b, const char *c, const struct tm *d);
char *strptime(const char *a, const char *b, struct tm *c);
time_t time(time_t *a);
int timer_create(clockid_t a, sigevent_t *b, timer_t *c);
int timer_delete(timer_t a);
int timer_gettime(timer_t a, struct itimerspec *b);
int timer_getoverrun(timer_t a);
int timer_settime(timer_t a, int b, const struct itimerspec * c, struct itimerspec *d);
void tzset(void);

#endif
