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
#include <errno.h>
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

char    *tzname[2] = { "UTC", "UTC" };
long int timezone = 0;
int      daylight = 0;

char *asctime(const struct tm *timeptr) {
    char *wday_name[] = {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };

    char *mon_name[] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    static char buf[26];
    sprintf(buf, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
            wday_name[timeptr->tm_wday],
            mon_name[timeptr->tm_mon],
            timeptr->tm_mday,
            timeptr->tm_hour,
            timeptr->tm_min,
            timeptr->tm_sec,
            1900 + timeptr->tm_year);

    return buf;
}

clock_t clock(void) {
    return syscall_process_run_time(syscall_process_pid());
}

int clock_getres(clockid_t clock_id, struct timespec *res) {
    return (PROFAN_FNI, 0);
}

int clock_gettime(clockid_t clock_id, struct timespec *tp) {
    static struct timespec g_first_time = {
        .tv_nsec = -1 // represents syscall_timer_get_ms
    };

    if (tp == NULL) {
        errno = EFAULT;
        return -1;
    }

    if (clock_id == CLOCK_MONOTONIC) {
        uint32_t ms = syscall_timer_get_ms();

        tp->tv_sec = ms / 1000;
        tp->tv_nsec = (ms % 1000) * 1000000;

        return 0;
    }

    if (clock_id != CLOCK_REALTIME) {
        errno = EINVAL;
        return -1;
    }

    if (g_first_time.tv_nsec == -1) {
        g_first_time.tv_sec = time(NULL);
        g_first_time.tv_nsec = syscall_timer_get_ms();

        tp->tv_sec = g_first_time.tv_sec;
        tp->tv_nsec = (g_first_time.tv_nsec % 1000) * 1000000;

        return 0;
    }

    uint32_t ms = syscall_timer_get_ms();

    tp->tv_sec = g_first_time.tv_sec + (ms / 1000 - g_first_time.tv_nsec / 1000);
    tp->tv_nsec = (ms % 1000) * 1000000;

    return 0;
}

int clock_settime(clockid_t clock_id, const struct timespec *tp) {
    return (PROFAN_FNI, 0);
}

char *ctime(const time_t *timer) {
    return asctime(localtime(timer));
}

double difftime(time_t end_time, time_t start_time) {
    return (PROFAN_FNI, 0);
}

struct tm *getdate(const char *string) {
    return (PROFAN_FNI, NULL);
}

struct tm *gmtime_r(const time_t *timer, struct tm *buf) {
    time_t secs_this_year;
    time_t t = *timer;

    buf->tm_sec  = 0;
    buf->tm_min  = 0;
    buf->tm_hour = 0;
    buf->tm_mday = 1;
    buf->tm_mon  = 0;
    buf->tm_year = 70;
    buf->tm_wday = (t / SEC_IN_DAY + 4) % 7; // 01.01.70 was Thursday
    buf->tm_isdst = 0;

    // This loop handles dates after 1970
    while (t >= (secs_this_year = is_leap(buf->tm_year) ? SEC_IN_LEAP_YEAR : SEC_IN_YEAR)) {
        t -= secs_this_year;
        buf->tm_year++;
    }

    // This loop handles dates before 1970
    while (t < 0)
        t += is_leap(--buf->tm_year) ? SEC_IN_LEAP_YEAR : SEC_IN_YEAR;

    buf->tm_yday = t / SEC_IN_DAY;    // days since Jan 1

    if (is_leap(buf->tm_year))
        day_in_month[1]++;

    while (t >= day_in_month[buf->tm_mon] * SEC_IN_DAY)
        t -= day_in_month[buf->tm_mon++] * SEC_IN_DAY;

    if (is_leap(buf->tm_year))        //  restore Feb
        day_in_month[1]--;

    while (t >= SEC_IN_DAY) {
        t -= SEC_IN_DAY;
        buf->tm_mday++;
    }

    while (t >= SEC_IN_HOUR) {
        t -= SEC_IN_HOUR;
        buf->tm_hour++;
    }

    while (t >= SEC_IN_MIN) {
        t -= SEC_IN_MIN;
        buf->tm_min++;
    }

    buf->tm_sec = t;
    return buf;
}

struct tm *gmtime(const time_t *tp) {
    static struct tm buf;
    return gmtime_r(tp, &buf);
}

struct tm *localtime_r(const time_t *timer, struct tm *buf) {
    return gmtime_r(timer, buf);
}

struct tm *localtime(const time_t *timer) {
    return gmtime(timer);
}

time_t mktime(struct tm *time) {
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

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (req == NULL || req->tv_sec < 0 || req->tv_nsec < 0 ||
            req->tv_nsec > 999999999) {
        errno = EINVAL;
        return -1;
    }

    syscall_process_sleep(syscall_process_pid(), req->tv_sec * 1000 + req->tv_nsec / 1000000);

    if (rem != NULL) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }

    return 0;
}

size_t strftime(char *str, size_t maxsize, const char *format, const struct tm *timeptr) {
    return (PROFAN_FNI, 0);
}

char *strptime(const char *buf, const char *format, struct tm *timeptr) {
    return (PROFAN_FNI, NULL);
}

time_t time(time_t *tloc) {
    struct tm time;
    syscall_time_get(&time);

    if (tloc != NULL)
        *tloc = mktime(&time);

    return mktime(&time);
}

int timer_create(clockid_t clock_id, sigevent_t *sevp, timer_t *timerid) {
    return (PROFAN_FNI, 0);
}

int timer_delete(timer_t timerid) {
    return (PROFAN_FNI, 0);
}

int timer_gettime(timer_t timerid, struct itimerspec *curr_value) {
    return (PROFAN_FNI, 0);
}

int timer_getoverrun(timer_t timerid) {
    return (PROFAN_FNI, 0);
}

int timer_settime(timer_t timerid, int flags, const struct itimerspec *c, struct itimerspec *d) {
    return (PROFAN_FNI, 0);
}

void tzset(void) {
    // timezone not implemented yet
}
