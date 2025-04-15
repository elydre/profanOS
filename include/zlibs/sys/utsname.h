/*****************************************************************************\
|   === utsname.h : 2025 ===                                                  |
|                                                                             |
|    Implementation of the sys/utsname.h header file from libC     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#include <profan/minimal.h>

_BEGIN_C_FILE

struct utsname {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
};

int uname(struct utsname *buf);

_END_C_FILE

#endif
