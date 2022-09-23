#ifndef TIME_H
#define TIME_H

#include <driver/rtc.h>
#include <stdint.h>

int time_calc_unix(time_t *time);
int time_gen_unix();

void sleep(int seconds);
void ms_sleep(uint32_t ms);

void time_gen_boot();
int time_get_boot();

#endif
