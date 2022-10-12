#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

uint32_t timer_get_tick();
void timer_init(uint32_t freq);

#endif
