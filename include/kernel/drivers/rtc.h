/****** This file is part of profanOS **************************\
|   == rtc.h ==                                      .pi0iq.    |
|                                                   d"  . `'b   |
|                                                   q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef RTC_H
#define RTC_H

#include <ktype.h>

void time_get(tm_t *time);
int  rtc_init(void);

#endif
