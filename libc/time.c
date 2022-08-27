#include <time.h>
#include <rtc.h>


int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int calc_unix_time(time_t *time) {
    int seconde_in_year = 31536000;
    int seconde_in_leap_year = 31622400;
    int seconde_in_month[12] = {0, 2678400, 5097600, 7776000, 10368000, 13046400, 15638400, 18316800, 20995200, 23587200, 26265600, 28857600};
    int seconde_in_day = 86400;
    int seconde_in_hour = 3600;
    int seconde_in_minute = 60;

    int start_year = 1970;
    int century = 2000;
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

int gen_unix_time() {
    time_t time;
    get_time(&time);
    return calc_unix_time(&time);
}

void sleep(int seconds) {
    int start_time = gen_unix_time();
    while (gen_unix_time() - start_time < seconds);
}
