/*****************************************************************************\
|   === strings.h : 2025 ===                                                  |
|                                                                             |
|    Implementation of the strings.h header file from libC         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _STRINGS_H
#define _STRINGS_H

#include <profan/minimal.h>
#include <stddef.h>

_BEGIN_C_FILE

int    bcmp(const void *s1, const void *s2, size_t n);
void   bcopy(const void *src, void *dest, size_t n);
void   bzero(void *s, size_t n);
int    ffs(int i);
int    ffsll(long long int i);
char  *index(const char *s, int c);
char  *rindex(const char *s, int c);
int    strcasecmp(const char *s1, const char *s2);
char  *strcasestr(const char *s1, const char *s2);
int    strncasecmp(const char *s1, const char *s2, size_t n);

_END_C_FILE

#endif
