/*****************************************************************************\
|   === utsname.c : 2025 ===                                                  |
|                                                                             |
|    Implementation of sys/utsname function from libC              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>

#include <sys/utsname.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int uname(struct utsname *buf) {
    char kernel_info[64];

    syscall_sys_kinfo(kernel_info, 64);

    char *space = strchr(kernel_info, ' ');

    strncpy(buf->version, kernel_info, space - kernel_info);
    buf->version[space - kernel_info] = '\0';
    strcpy(buf->release, space + 1);
    strcpy(buf->machine, "i386");

    char *sysname = getenv("UNAME_KERNEL");

    if (sysname == NULL) {
        strcpy(buf->sysname, "profan");
        strcpy(buf->nodename, "( ._.)");
    } else {
        strlcpy(buf->sysname, sysname, sizeof(buf->sysname));
        strcpy(buf->nodename, "( ^-^)");
    }

    return 0;
}
