/*****************************************************************************\
|   === string.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the string.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

char  *basename(const char *path);
void   bcopy(const void *s2, void *s1, size_t n);
void   bzero(void *s, size_t n);
char  *dirname(char *path);
int    ffs(int i);
int    ffsll(long long int i);
void  *memccpy(void *s1, const void *s2, int c, size_t n);
void  *memchr(const void *s, int c, size_t n);
int    memcmp(const void *s1, const void *s2, size_t n);
void  *memcpy(void *s1, const void *s2, size_t n);
void  *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen);
void  *memmove(void *s1, const void *s2, size_t n);
void  *mempcpy(void *s1, const void *s2, size_t n);
void  *memrchr(const void *s, int c, size_t n);
void  *memset(void *s, int c, size_t n);
void   psignal(int signum, const char *message);
void  *rawmemchr(const void *s, int c);
char  *stpcpy(char *s1, const char *s2);
char  *stpncpy(char *s1, const char *s2, size_t n);
int    strcasecmp (const char *s1, const char *s2);
char  *strcasestr(const char *s1, const char *s2);
char  *strcat(char *s1, const char *s2);
char  *strchr(const char *p, int ch);
char  *strchrnul(const char *s, int c);
int    strcmp(const char *s1, const char *s2);
char  *strcpy(char *s1, const char *s2);
size_t strcspn(const char *s1, const char *s2);
char  *strdup(const char *s);
char  *strerror(int errnum);
size_t strlcat(char *dst, const char *src, size_t n);
size_t strlcpy(char *dst, const char *src, size_t n);
size_t strlen(const char *s);
int    strncasecmp(const char *s1, const char *s2, size_t n);
char  *strncat(char *s1, const char *s2, size_t n);
int    strncmp(const char *s1, const char *s2, size_t n);
char  *strncpy(char *s1, const char *s2, size_t n);
char  *strndup(const char *s1, size_t n);
size_t strnlen(const char *s, size_t max);
char  *strpbrk(const char *s1, const char *s2);
char  *strrchr(const char *s, int c);
char  *strsep(char **s1, const char *s2);
char  *strsignal(int signum);
size_t strspn(const char *s, const char *c);
char  *strstr(const char *string, const char *substring);
char  *strtok(char *s1, const char *s2);
char  *strtok_r(char *s1, const char *s2, char **next_start);
int    strverscmp(const char *s1, const char *s2);

#endif
