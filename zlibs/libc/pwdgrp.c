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
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#include "config_libc.h"

struct group *getgrgid(gid_t gid) {
    if (gid != 0) {
        errno = ENOENT;
        return NULL;
    }

    static struct group root = {
        .gr_name = "root",
        .gr_passwd = "x",
        .gr_gid = 0,
        .gr_mem = NULL
    };

    return &root;
}

struct group *getgrnam(const char *) {
    return (PROFAN_FNI, NULL);
}

struct passwd *getpwuid(uid_t uid) {
    if (uid != 0) {
        errno = ENOENT;
        return NULL;
    }

    static struct passwd root = {
        .pw_name = "root",
        .pw_passwd = "x",
        .pw_uid = 0,
        .pw_gid = 0,
        .pw_gecos = "root",
        .pw_dir = "/user",
        .pw_shell = SYSTEM_SHELL_PATH
    };

    return &root;
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
