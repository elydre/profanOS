#ifndef STRING_H
#define STRING_H

#include <type.h>

char  *basename(const char *path);
void   bcopy(const void *s2, void *s1, size_t n);
void   bzero(void *s, size_t n);
char  *dirname(char *path);
int    ffs(int i);
int    ffsll(long long int i);
void  *memccpy(void *restrict s1, const void *restrict s2, int c, size_t n);
void  *memchr(const void *s, int c, size_t n);
int    memcmp(const void *s1, const void *s2, size_t n);
void  *memcpy(void *restrict s1, const void *restrict s2, size_t n);
void  *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen);
void  *memmove(void *s1, const void *s2, size_t n);
void  *mempcpy(void *restrict s1, const void *restrict s2, size_t n);
void  *memrchr(const void *s, int c, size_t n);
void  *memset(void *s, int c, size_t n);
void   psignal(int signum, register const char *message);
void  *rawmemchr(const void *s, int c);
char  *stpcpy(register char *restrict s1, const char *restrict s2);
char  *stpncpy(register char *restrict s1, register const char *restrict s2, size_t n);
int    strcasecmp (const char *s1, const char *s2);
int    strcasecmp_l(register const char *s1, register const char *s2, locale_t loc);
char  *strcasestr(const char *s1, const char *s2);
char  *strcat(char *restrict s1, register const char *restrict s2);
char  *strchr(const char *p, int ch);
char  *strchrnul(register const char *s, int c);
int    strcmp(register const char *s1, register const char *s2);
char  *strcpy(char *restrict s1, const char *restrict s2);
size_t strcspn(const char *s1, const char *s2);
size_t strlen(const char *s);
char  *strdup(register const char *s);
char  *strerror(int errnum);
size_t strlcat(register char *restrict dst, register const char *restrict src, size_t n);
size_t strlcpy(register char *restrict dst, register const char *restrict src, size_t n);
size_t strlen(const char *s);
int    strncasecmp(register const char *s1, register const char *s2, size_t n);
int    strncasecmp_l(register const char *s1, register const char *s2, size_t n, locale_t loc);
char  *strncat(char *restrict s1, register const char *restrict s2, size_t n);
int    strncmp(register const char *s1, register const char *s2, size_t n);
char  *strncpy(char *restrict s1, register const char *restrict s2, size_t n);
char  *strndup(register const char *s1, size_t n);
size_t strnlen(const char *s, size_t max);
char  *strpbrk(const char *s1, const char *s2);
char  *strrchr(register const  char *s, int c);
char  *strsep(char **restrict s1, const char *restrict s2);
char  *strsignal(int signum);
size_t strspn(const char *s, const char *c);
char  *strstr(register char *string, char *substring);
char  *strtok(char *restrict s1, const char *restrict s2);
char  *strtok_r(char *restrict s1, const char *restrict s2, char **restrict next_start);
int    strverscmp(const char *s1, const char *s2);

#endif
