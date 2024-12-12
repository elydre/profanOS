/*****************************************************************************\
|   === time.c : 2024 ===                                                     |
|                                                                             |
|    Implementation of time functions from libC                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define is_leap_year(year) ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)

static uint32_t seconde_in_month[] = {
    0,
    2678400,
    5097600,
    7776000,
    10368000,
    13046400,
    15638400,
    18316800,
    20995200,
    23587200,
    26265600,
    28857600
};

#define seconde_in_year 31536000
#define seconde_in_leap_year 31622400
#define seconde_in_day 86400
#define seconde_in_hour 3600
#define seconde_in_minute 60
#define start_year 1970
#define century 2000

char *asctime(const tm_t *a) {
    profan_nimpl("asctime");
    return NULL;
}

char *asctime_r(const tm_t *a, char *b) {
    profan_nimpl("asctime_r");
    return NULL;
}

clock_t clock(void) {
    profan_nimpl("clock");
    return 0;
}

int clock_getres(clockid_t a, timespec_t *n) {
    profan_nimpl("clock_getres");
    return 0;
}

int clock_gettime(clockid_t a, timespec_t *b) {
    profan_nimpl("clock_gettime");
    return 0;
}

int clock_settime(clockid_t a, const timespec_t *b) {
    profan_nimpl("clock_settime");
    return 0;
}

char *ctime(const time_t *a) {
    profan_nimpl("ctime");
    return NULL;
}

char *ctime_r(const time_t *a, char *b) {
    profan_nimpl("ctime_r");
    return NULL;
}

double difftime(time_t a, time_t b) {
    profan_nimpl("difftime");
    return 0;
}

tm_t *getdate(const char *a) {
    profan_nimpl("getdate");
    return NULL;
}

tm_t *gmtime(const time_t *a) {
    profan_nimpl("gmtime");
    return NULL;
}

tm_t *gmtime_r(const time_t *a, tm_t *b) {
    profan_nimpl("gmtime_r");
    return NULL;
}

tm_t *localtime(const time_t *a) {
    tm_t *time = malloc(sizeof(tm_t));
    syscall_time_get(time);
    return time;
}

tm_t *localtime_r(const time_t *a, tm_t *b) {
    profan_nimpl("localtime_r");
    return NULL;
}

time_t mktime(tm_t *time) {
    time_t unix_time = 0;

    // Add seconds
    unix_time += time->tm_sec;

    // Add minutes
    unix_time += time->tm_min * seconde_in_minute;

    // Add hours
    unix_time += time->tm_hour * seconde_in_hour;

    // Add days
    unix_time += (time->tm_mday - 1) * seconde_in_day;

    // Add months
    unix_time += seconde_in_month[time->tm_mon - 1];

    // Add years
    for (int i = start_year; i < time->tm_year + century; i++) {
        unix_time += is_leap_year(i) ? seconde_in_leap_year : seconde_in_year;
    }

    return unix_time;
}

int nanosleep(const timespec_t *a, timespec_t *b) {
    profan_nimpl("nanosleep");
    return 0;
}

size_t strftime(char *a, size_t b, const char *c, const tm_t *d) {
    profan_nimpl("strftime");
    return 0;
}

char *strptime(const char *a, const char *b, tm_t *c) {
    profan_nimpl("strptime");
    return NULL;
}

time_t time(time_t *a) {
    tm_t time;
    syscall_time_get(&time);

    if (a != NULL) {
        *a = mktime(&time);
    }
    return mktime(&time);
}

int timer_create(clockid_t a, sigevent_t *b, timer_t *c) {
    profan_nimpl("timer_create");
    return 0;
}

int timer_delete(timer_t a) {
    profan_nimpl("timer_delete");
    return 0;
}

int timer_gettime(timer_t a, itimerspec_t *b) {
    profan_nimpl("timer_gettime");
    return 0;
}

int timer_getoverrun(timer_t a) {
    profan_nimpl("timer_getoverrun");
    return 0;
}

int timer_settime(timer_t a, int b, const itimerspec_t * c, itimerspec_t *d) {
    profan_nimpl("timer_settime");
    return 0;
}

void tzset(void) {
    profan_nimpl("tzset");
}
