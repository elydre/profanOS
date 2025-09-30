/*****************************************************************************\
|   === ktype.h : 2024 ===                                                    |
|                                                                             |
|    Kernel types header                                           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef KTYPE_H
#define KTYPE_H

#ifndef FS_SECTOR_SIZE
#define FS_SECTOR_SIZE 512
#endif

typedef struct tm {
    int    tm_sec;   // seconds [0,61]
    int    tm_min;   // minutes [0,59]
    int    tm_hour;  // hour [0,23]
    int    tm_mday;  // day of month [1,31]
    int    tm_mon;   // month of year [0,11]
    int    tm_year;  // years since 1900
    int    tm_wday;  // day of week [0,6] (Sunday = 0)
    int    tm_yday;  // day of year [0,365]
    int    tm_isdst; // daylight savings flag
} tm_t;

#ifndef NULL
#define NULL ((void *) 0)
#endif

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// va_list
typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v, l)
#define va_arg(v, l)   __builtin_va_arg(v, l)
#define va_end(v)      __builtin_va_end(v)

#endif
