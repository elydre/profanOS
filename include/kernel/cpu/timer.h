/*****************************************************************************\
|   === timer.h : 2024 ===                                                    |
|                                                                             |
|    Kernel Timer (PIT) header                                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef TIMER_H
#define TIMER_H

#include <ktype.h>

extern uint32_t TIMER_TICKS;

int      timer_init(void);
uint32_t timer_get_ms(void);

#endif
