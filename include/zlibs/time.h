#ifndef TIME_H
#define TIME_H

#include <type.h>

// Number of clock ticks per second returned by the times() function (LEGACY).
#define CLK_TCK 100

// A number used to convert the value returned by the clock() function into seconds.
#define CLOCKS_PER_SEC 1000

typedef struct timespec {
    time_t  tv_sec;  // seconds
    long    tv_nsec; // nanoseconds
} timespec_t;

typedef struct itimerspec {
    timespec_t it_interval; // timer period
    timespec_t it_value;    // timer expiration
} itimerspec_t;

// The identifier of the systemwide realtime clock.
#define CLOCK_REALTIME 0
// Flag indicating time is absolute with respect to the clock associated with a timer.
#define TIMER_ABSTIME 1

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
