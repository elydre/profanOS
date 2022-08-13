#ifndef TIME_H
#define TIME_H

#include "../drivers/rtc.h"

int is_leap_year(int year);
int calc_unix_time(time_t *time);
int gen_unix_time();
void sleep(int seconds);

#endif
