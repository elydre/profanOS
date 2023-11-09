#ifndef TIMER_H
#define TIMER_H

#include <ktype.h>

int      timer_init(void);

uint32_t timer_get_ms(void);
uint32_t timer_get_ticks(void);

#endif
