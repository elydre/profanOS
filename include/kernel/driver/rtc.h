#ifndef RTC_H
#define RTC_H

typedef struct {
    int seconds;
    int minutes;
    int hours;
    int day_of_week;
    int day_of_month;
    int month;
    int year;
    int full[6];
} time_t;
 
void time_get(time_t *time);
void rtc_init();

#endif
