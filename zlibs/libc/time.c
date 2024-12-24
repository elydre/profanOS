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

#define IS_LEAP_YEAR(year) (((year) % 4 == 0 && (year) % 100 != 0) || (year) % 400 == 0)

#define CENTURY          2000

#define SEC_IN_MIN       (60)
#define SEC_IN_HOUR      (60 * SEC_IN_MIN)
#define SEC_IN_DAY       (24 * SEC_IN_HOUR)
#define SEC_IN_YEAR      (365 * SEC_IN_DAY)
#define SEC_IN_LEAP_YEAR (366 * SEC_IN_DAY)

static uint32_t cumulated_seconds_in_month[] = {
    SEC_IN_DAY * 0,   // January
    SEC_IN_DAY * 31,  // February
    SEC_IN_DAY * 59,  // March
    SEC_IN_DAY * 90,  // April
    SEC_IN_DAY * 120, // May
    SEC_IN_DAY * 151, // June
    SEC_IN_DAY * 181, // July
    SEC_IN_DAY * 212, // August
    SEC_IN_DAY * 243, // September
    SEC_IN_DAY * 273, // October
    SEC_IN_DAY * 304, // November
    SEC_IN_DAY * 334, // December
};

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
    profan_nimpl("localtime");
    return NULL;
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
    unix_time += time->tm_min * SEC_IN_MIN;

    // Add hours
    unix_time += time->tm_hour * SEC_IN_HOUR;

    // Add days
    unix_time += (time->tm_mday - 1) * SEC_IN_DAY;

    // Add months
    unix_time += cumulated_seconds_in_month[time->tm_mon - 1];

    // Add leap day
    if (IS_LEAP_YEAR(time->tm_year + CENTURY) && time->tm_mon > 2)
        unix_time += SEC_IN_DAY;

    // Add years
    for (int i = 1970; i < time->tm_year + CENTURY; i++)
        unix_time += IS_LEAP_YEAR(i) ? SEC_IN_LEAP_YEAR : SEC_IN_YEAR;

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
