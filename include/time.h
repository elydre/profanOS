#ifndef TIME_H
#define TIME_H

#include <driver/rtc.h>
#include <stdint.h>

int is_leap_year(int year);
int calc_unix_time(time_t *time);
int gen_unix_time();
void sleep(int seconds);
void ms_sleep(uint32_t ms);

#endif
