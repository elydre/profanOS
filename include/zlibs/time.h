#ifndef TIME_ID
#define TIME_ID 1012

#include <type.h>

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

// Null pointer constant.
#ifndef NULL
#define NULL 0
#endif

// Number of clock ticks per second returned by the times() function (LEGACY).
#define CLK_TCK 100
// A number used to convert the value returned by the clock() function into seconds.
#define CLOCKS_PER_SEC 1000

struct tm {
    int    tm_sec ;  //seconds [0,61]
    int    tm_min ;  //minutes [0,59]
    int    tm_hour;  //hour [0,23]
    int    tm_mday;  //day of month [1,31]
    int    tm_mon ;  //month of year [0,11]
    int    tm_year;  //years since 1900
    int    tm_wday;  //day of week [0,6] (Sunday = 0)
    int    tm_yday;  //day of year [0,365]
    int    tm_isdst; //daylight savings flag
};

typedef struct tm tm_t;

typedef struct timespec_t {
    time_t  tv_sec ; // seconds
    long    tv_nsec; // nanoseconds
} timespec_t;

typedef struct itimerspec_t {
    timespec_t it_interval; // timer period
    timespec_t it_value;    // timer expiration
} itimerspec_t;

// The identifier of the systemwide realtime clock.
#define CLOCK_REALTIME 0
// Flag indicating time is absolute with respect to the clock associated with a timer.
#define TIMER_ABSTIME 1

#ifndef TIME_C
#define asctime ((char * (*)(const tm_t *)) (get_func_addr(TIME_ID, 2)))
#define asctime_r ((char * (*)(const tm_t *, char *)) (get_func_addr(TIME_ID, 3)))
#define clock ((clock_t (*)(void)) (get_func_addr(TIME_ID, 4)))
#define clock_getres ((int (*)(clockid_t, timespec_t *)) (get_func_addr(TIME_ID, 5)))
#define clock_gettime ((int (*)(clockid_t, timespec_t *)) (get_func_addr(TIME_ID, 6)))
#define clock_settime ((int (*)(clockid_t, const timespec_t *)) (get_func_addr(TIME_ID, 7)))
#define ctime ((char * (*)(const time_t *)) (get_func_addr(TIME_ID, 8)))
#define ctime_r ((char * (*)(const time_t *, char *)) (get_func_addr(TIME_ID, 9)))
#define difftime ((double (*)(time_t, time_t)) (get_func_addr(TIME_ID, 10)))
#define getdate ((tm_t * (*)(const char *)) (get_func_addr(TIME_ID, 11)))
#define gmtime ((tm_t * (*)(const time_t *)) (get_func_addr(TIME_ID, 12)))
#define gmtime_r ((tm_t * (*)(const time_t *, tm_t *)) (get_func_addr(TIME_ID, 13)))
#define localtime ((tm_t * (*)(const time_t *)) (get_func_addr(TIME_ID, 14)))
#define localtime_r ((tm_t * (*)(const time_t *, tm_t *)) (get_func_addr(TIME_ID, 15)))
#define mktime ((time_t (*)(tm_t *)) (get_func_addr(TIME_ID, 16)))
#define nanosleep ((int (*)(const timespec_t *, timespec_t *)) (get_func_addr(TIME_ID, 17)))
#define strftime ((size_t (*)(char *, size_t, const char *, const tm_t *)) (get_func_addr(TIME_ID, 18)))
#define strptime ((char * (*)(const char *, const char *, tm_t *)) (get_func_addr(TIME_ID, 19)))
#define time ((time_t (*)(time_t *)) (get_func_addr(TIME_ID, 20)))
#define timer_create ((int (*)(clockid_t, sigevent_t *, timer_t *)) (get_func_addr(TIME_ID, 21)))
#define timer_delete ((int (*)(timer_t)) (get_func_addr(TIME_ID, 22)))
#define timer_gettime ((int (*)(timer_t, itimerspec_t *)) (get_func_addr(TIME_ID, 23)))
#define timer_getoverrun ((int (*)(timer_t)) (get_func_addr(TIME_ID, 24)))
#define timer_settime ((int (*)(timer_t, int, const itimerspec_t *, itimerspec_t *)) (get_func_addr(TIME_ID, 25)))
#define tzset ((void (*)(void)) (get_func_addr(TIME_ID, 26)))
#endif

#endif
