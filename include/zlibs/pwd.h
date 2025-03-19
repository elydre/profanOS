/*****************************************************************************\
|   === pwd.h : 2025 ===                                                      |
|                                                                             |
|    Implementation of the pwd.h header file from libC             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PWD_H
#define _PWD_H

#include <sys/types.h>

struct passwd {
    char   *pw_name;       // user name
    char   *pw_passwd;     // encrypted password
    uid_t   pw_uid;        // user uid
    gid_t   pw_gid;        // user gid
    time_t  pw_change;     // password change time
    char   *pw_class;      // user access class
    char   *pw_gecos;      // Honeywell login info
    char   *pw_dir;        // home directory
    char   *pw_shell;      // default shell
    time_t  pw_expire;     // account expiration
};

struct passwd *getpwuid(uid_t);
struct passwd *getpwnam(const char *);
struct passwd *getpwuid_shadow(uid_t);
struct passwd *getpwnam_shadow(const char *);

#endif
