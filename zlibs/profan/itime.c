#include <syscall.h>
#include <profan.h>

#define seconde_in_year 31536000
#define seconde_in_leap_year 31622400
#define seconde_in_day 86400
#define seconde_in_hour 3600
#define seconde_in_minute 60
#define start_year 1970
#define century 2000

static int boot_time;
static int seconde_in_month[] = {
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

int main() {
    return 0;
}

int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int time_calc_unix(i_time_t *time) {
    int unix_time = 0;

    for (int i = start_year; i < time->full[5] + century; i++) {
        unix_time += is_leap_year(i) ? seconde_in_leap_year : seconde_in_year;
    }

    unix_time += seconde_in_month[time->full[4] - 1];
    unix_time += is_leap_year(time->full[5] + century) ? seconde_in_year : 0;
    unix_time += (time->full[3] - 1) * seconde_in_day;
    unix_time += time->full[2] * seconde_in_hour;
    unix_time += time->full[1] * seconde_in_minute;
    unix_time += time->full[0];
    return unix_time;
}

int time_gen_unix() {
    i_time_t time;
    c_time_get(&time);
    return time_calc_unix(&time);
}

void time_add(i_time_t *time, int seconde) {
    // add seconde to current time
    time->full[0] += seconde;
    while (time->full[0] >= seconde_in_minute) {
        time->full[0] -= seconde_in_minute;
        time->full[1]++;
    }
    while (time->full[1] >= 60) {
        time->full[1] -= 60;
        time->full[2]++;
    }
    while (time->full[2] >= 24) {
        time->full[2] -= 24;
        time->full[3]++;
    }

    int year = time->full[5] + century;
    while (time->full[3] > (is_leap_year(year) ? 366 : 365)) {
        time->full[3] -= is_leap_year(year) ? 366 : 365;
        time->full[5]++;
        year++;
    }

    int month = time->full[4];
    while (time->full[3] > (is_leap_year(year) ? seconde_in_leap_year : seconde_in_year) / seconde_in_day) {
        time->full[3] -= (is_leap_year(year) ? seconde_in_leap_year : seconde_in_year) / seconde_in_day;
        time->full[5]++;
        year++;
    }
    while (time->full[3] > seconde_in_month[month] / seconde_in_day) {
        time->full[3] -= seconde_in_month[month] / seconde_in_day;
        time->full[4]++;
        month++;
    }
}

void ms_sleep(uint32_t ms) {
    if (ms <= 0) return;
    c_process_sleep(c_process_get_pid(), ms);
}

void ms_sleep_perfect(uint32_t ms) {
    if (ms <= 0) return;
    int debut = c_timer_get_ms();
    if (ms > 10) {
        c_process_sleep(c_process_get_pid(), ms - 10);
    }
    while (c_timer_get_ms() - debut < ms);
}
