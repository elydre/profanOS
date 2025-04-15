/*****************************************************************************\
|   === grp.h : 2025 ===                                                      |
|                                                                             |
|    Implementation of the grp.h header file from libC             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _GRP_H
#define _GRP_H

#include <profan/minimal.h>
#include <sys/types.h>

_BEGIN_C_FILE

struct group {
    char    *gr_name;       // group name
    char    *gr_passwd;     // group password
    gid_t    gr_gid;        // group id
    char   **gr_mem;        // group members
};

struct group *getgrgid(gid_t);
struct group *getgrnam(const char *);

_END_C_FILE

#endif
