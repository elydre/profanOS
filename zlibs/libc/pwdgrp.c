/*****************************************************************************\
|   === pwdgrp.c : 2025 ===                                                   |
|                                                                             |
|    Implementation of pwd and grp functions from libC             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan.h>
#include <stddef.h>
#include <pwd.h>
#include <grp.h>

struct group *getgrgid(gid_t) {
    return (PROFAN_FNI, NULL);
}

struct group *getgrnam(const char *) {
    return (PROFAN_FNI, NULL);
}

struct passwd *getpwuid(uid_t) {
    return (PROFAN_FNI, NULL);
}

struct passwd *getpwnam(const char *) {
    return (PROFAN_FNI, NULL);
}

struct passwd *getpwuid_shadow(uid_t) {
    return (PROFAN_FNI, NULL);
}

struct passwd *getpwnam_shadow(const char *) {
    return (PROFAN_FNI, NULL);
}
