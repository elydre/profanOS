#ifndef TIME_H
#define TIME_H

#include <driver/rtc.h>
#include <stdint.h>

int time_calc_unix(time_t *time);
int time_gen_unix();

void time_add(time_t *time, int seconde);
void time_jet_lag(time_t *time);

void time_gen_boot();
int time_get_boot();

void sleep(int seconds);
void ms_sleep(uint32_t ms);

#endif
