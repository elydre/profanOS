#ifndef TIME_ID
#define TIME_ID 1012

#include <type.h>

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
int main();
*/

// Null pointer constant.
#define NULL 0
// Number of clock ticks per second returned by the times() function (LEGACY).
#define CLK_TCK 100
// A number used to convert the value returned by the clock() function into seconds.
#define CLOCKS_PER_SEC 1000

typedef struct tm_t {
    int    tm_sec ;  //seconds [0,61]
    int    tm_min ;  //minutes [0,59]
    int    tm_hour;  //hour [0,23]
    int    tm_mday;  //day of month [1,31]
    int    tm_mon ;  //month of year [0,11]
    int    tm_year;  //years since 1900
    int    tm_wday;  //day of week [0,6] (Sunday = 0)
    int    tm_yday;  //day of year [0,365]
    int    tm_isdst; //daylight savings flag
} tm_t;

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

#ifndef TIME_c



#endif

#endif
