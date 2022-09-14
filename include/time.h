#ifndef TIME_H
#define TIME_H

#include <driver/rtc.h>
#include <stdint.h>

int calc_unix_time(time_t *time);
int gen_unix_time();

void sleep(int seconds);
void ms_sleep(uint32_t ms);

void gen_boot_time();
int get_boot_time();

#endif
