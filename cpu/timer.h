#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

uint32_t timer_get_tick();
void init_timer(uint32_t freq);
void timer_get_refresh_time(int target[5]);

#endif
