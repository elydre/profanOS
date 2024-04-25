/****** This file is part of profanOS **************************\
|   == timer.h ==                                    .pi0iq.    |
|                                                   d"  . `'b   |
|   Kernel Timer (PIT) header                       q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef TIMER_H
#define TIMER_H

#include <ktype.h>

int      timer_init(void);

uint32_t timer_get_ms(void);
uint32_t timer_get_ticks(void);

#endif
