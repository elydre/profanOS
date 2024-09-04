/*****************************************************************************\
|   === rtc.h : 2024 ===                                                      |
|                                                                             |
|    Kernel RTC driver header                                      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef RTC_H
#define RTC_H

#include <ktype.h>

int time_get(tm_t *time);
int rtc_init(void);

#endif

