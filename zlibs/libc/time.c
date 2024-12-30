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

static int day_in_month[] = {
    31, // January
    28, // February
    31, // March
    30, // April
    31, // May
    30, // June
    31, // July
    31, // August
    30, // September
    31, // October
    30, // November
    31, // December
};

static inline int is_leap(int year) {
    year += 1900;
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

char *asctime(const tm_t *a) {
    profan_nimpl("asctime");
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

double difftime(time_t a, time_t b) {
    profan_nimpl("difftime");
    return 0;
}

tm_t *getdate(const char *a) {
    profan_nimpl("getdate");
    return NULL;
}

tm_t *gmtime(const time_t *tp) {
    static tm_t tm;
    time_t t, secs_this_year;

    t = *tp;
    tm.tm_sec  = 0;
    tm.tm_min  = 0;
    tm.tm_hour = 0;
    tm.tm_mday = 1;
    tm.tm_mon  = 0;
    tm.tm_year = 70;
    tm.tm_wday = (t / SEC_IN_DAY + 4) % 7; // 01.01.70 was Thursday
    tm.tm_isdst = 0;

    // This loop handles dates after 1970
    while (t >= (secs_this_year = is_leap(tm.tm_year) ? SEC_IN_LEAP_YEAR : SEC_IN_YEAR)) {
        t -= secs_this_year;
        tm.tm_year++;
    }

    // This loop handles dates before 1970
    while (t < 0)
        t += is_leap(--tm.tm_year) ? SEC_IN_LEAP_YEAR : SEC_IN_YEAR;

    tm.tm_yday = t / SEC_IN_DAY;    // days since Jan 1

    if (is_leap(tm.tm_year))
        day_in_month[1]++;

    while (t >= day_in_month[tm.tm_mon] * SEC_IN_DAY)
        t -= day_in_month[tm.tm_mon++] * SEC_IN_DAY;

    if (is_leap(tm.tm_year))        //  restore Feb
        day_in_month[1]--;

    while (t >= SEC_IN_DAY) {
        t -= SEC_IN_DAY;
        tm.tm_mday++;
    }

    while (t >= SEC_IN_HOUR) {
        t -= SEC_IN_HOUR;
        tm.tm_hour++;
    }

    while (t >= SEC_IN_MIN) {
        t -= SEC_IN_MIN;
        tm.tm_min++;
    }

    tm.tm_sec = t;
    return &tm;
}

tm_t *localtime(const time_t *a) {
    return gmtime(a);
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
    if (is_leap(time->tm_year + 100) && time->tm_mon > 2)
        unix_time += SEC_IN_DAY;

    // Add years
    for (int i = 70; i < time->tm_year + 100; i++)
        unix_time += is_leap(i) ? SEC_IN_LEAP_YEAR : SEC_IN_YEAR;

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
