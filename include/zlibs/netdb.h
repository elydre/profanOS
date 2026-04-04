/*****************************************************************************\
|   === netdb.h : 2026 ===                                                    |
|                                                                             |
|    Implementation of the netdb.h header file from libC           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _NETDB_H
#define _NETDB_H

#define HOST_NOT_FOUND 0xFF
#define NO_DATA        0xFE
#define NO_RECOVERY    0xFD
#define TRY_AGAIN      0xFC

struct hostent {
    char *h_name;
    char **h_aliases;
    int h_addrtype;
    int h_length;
    char **h_addr_list;
};

extern int h_errno;

struct hostent *gethostbyname(const char *name);

#endif
