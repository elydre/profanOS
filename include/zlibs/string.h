#ifndef STRING_ID
#define STRING_ID 1008

#include <type.h>

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
int main();
char *basename(const char *path);
void bcopy(const void *s2, void *s1, size_t n);
*/

#define basename(path) ((char *(*)(const char *)) get_func_addr(STRING_ID, 3))(path)
#define bcopy(s2, s1, n) ((void (*)(const void *, void *, size_t)) get_func_addr(STRING_ID, 4))(s2, s1, n)
#define bzero(s, n) ((void (*)(void *, size_t)) get_func_addr(STRING_ID, 5))(s, n)
#define dirname(path) ((char *(*)(const char *)) get_func_addr(STRING_ID, 6))(path)
#define ffs(i) ((int (*)(int)) get_func_addr(STRING_ID, 7))(i)
#define ffsll(i) ((int (*)(long long)) get_func_addr(STRING_ID, 8))(i)
#define memccpy(s1, s2, c, n) ((void *(*)(void *, const void *, int, size_t)) get_func_addr(STRING_ID, 9))(s1, s2, c, n)
#define memchr(s, c, n) ((uint8_t *(*)(const uint8_t *, uint8_t, size_t)) get_func_addr(STRING_ID, 10))(s, c, n)
#define memcmp(s1, s2, n) ((int (*)(const uint8_t *, const uint8_t *, size_t)) get_func_addr(STRING_ID, 11))(s1, s2, n)
#define memcpy(s1, s2, n) ((void *(*)(void *, const void *, size_t)) get_func_addr(STRING_ID, 12))(s1, s2, n)
#define memmem(haystack, hlen, needle, nlen) ((void *(*)(const void *, size_t, const void *, size_t)) get_func_addr(STRING_ID, 13))(haystack, hlen, needle, nlen)
#define memmove(s1, s2, n) ((void *(*)(void *, const void *, size_t)) get_func_addr(STRING_ID, 14))(s1, s2, n)
#define mempcpy(s1, s2, n) ((void *(*)(void *, const void *, size_t)) get_func_addr(STRING_ID, 15))(s1, s2, n)
#define memrchr(s, c, n) ((void *(*)(const void *, int, size_t)) get_func_addr(STRING_ID, 16))(s, c, n)
#define memset(s, c, n) ((void *(*)(void *, int, size_t)) get_func_addr(STRING_ID, 17))(s, c, n)
#define psignal(signum, message) ((void (*)(int, const char *)) get_func_addr(STRING_ID, 18))(signum, message)
#define rawmemchr(s, c) ((void *(*)(const void *, int)) get_func_addr(STRING_ID, 19))(s, c)
#define stpcpy(s1, s2) ((char *(*)(char *, const char *)) get_func_addr(STRING_ID, 20))(s1, s2)
#define stpncpy(s1, s2, n) ((char *(*)(char *, const char *, size_t)) get_func_addr(STRING_ID, 21))(s1, s2, n)
#define strcasecmp(s1, s2) ((int (*)(const char *, const char *)) get_func_addr(STRING_ID, 22))(s1, s2)
#define strcasecmp_l(s1, s2, loc) ((int (*)(const char *, const char *, locale_t)) get_func_addr(STRING_ID, 23))(s1, s2, loc)
#define strcasestr(s1, s2) ((char *(*)(const char *, const char *)) get_func_addr(STRING_ID, 24))(s1, s2)
#define strcat(s1, s2) ((char *(*)(char *, const char *)) get_func_addr(STRING_ID, 25))(s1, s2)
#define strchr(s, c) ((char *(*)(const char *, int)) get_func_addr(STRING_ID, 26))(s, c)
#define strchrnul(s, c) ((char *(*)(const char *, int)) get_func_addr(STRING_ID, 27))(s, c)
#define strcmp(s1, s2) ((int (*)(const char *, const char *)) get_func_addr(STRING_ID, 28))(s1, s2)
#define strcpy(s1, s2) ((char *(*)(char *, const char *)) get_func_addr(STRING_ID, 29))(s1, s2)
#define strcspn(s1, s2) ((size_t (*)(const char *, const char *)) get_func_addr(STRING_ID, 30))(s1, s2)
#define strdup(s) ((char *(*)(const char *)) get_func_addr(STRING_ID, 31))(s)
#define strerror(errnum) ((char *(*)(int)) get_func_addr(STRING_ID, 32))(errnum)
#define strlcat(s1, s2, n) ((size_t (*)(char *, const char *, size_t)) get_func_addr(STRING_ID, 33))(s1, s2, n)
#define strlcpy(s1, s2, n) ((size_t (*)(char *, const char *, size_t)) get_func_addr(STRING_ID, 34))(s1, s2, n)
#define strlen(s) ((size_t (*)(const char *)) get_func_addr(STRING_ID, 35))(s)
#define strncasecmp(s1, s2, n) ((int (*)(const char *, const char *, size_t)) get_func_addr(STRING_ID, 36))(s1, s2, n)
#define strncasecmp_l(s1, s2, n, loc) ((int (*)(const char *, const char *, size_t, locale_t)) get_func_addr(STRING_ID, 37))(s1, s2, n, loc)
#define strncat(s1, s2, n) ((char *(*)(char *, const char *, size_t)) get_func_addr(STRING_ID, 38))(s1, s2, n)
#define strncmp(s1, s2, n) ((int (*)(const char *, const char *, size_t)) get_func_addr(STRING_ID, 39))(s1, s2, n)
#define strncpy(s1, s2, n) ((char *(*)(char *, const char *, size_t)) get_func_addr(STRING_ID, 40))(s1, s2, n)
#define strndup(s, n) ((char *(*)(const char *, size_t)) get_func_addr(STRING_ID, 41))(s, n)
#define strnlen(s, n) ((size_t (*)(const char *, size_t)) get_func_addr(STRING_ID, 42))(s, n)
#define strpbrk(s1, s2) ((char *(*)(const char *, const char *)) get_func_addr(STRING_ID, 43))(s1, s2)
#define strrchr(s, c) ((char *(*)(const char *, int)) get_func_addr(STRING_ID, 44))(s, c)
#define strsep(s1, s2) ((char *(*)(char **, const char *)) get_func_addr(STRING_ID, 45))(s1, s2)
#define strsignal(signum) ((char *(*)(int)) get_func_addr(STRING_ID, 46))(signum)
#define strspn(s1, s2) ((size_t (*)(const char *, const char *)) get_func_addr(STRING_ID, 47))(s1, s2)
#define strstr(s1, s2) ((char *(*)(const char *, const char *)) get_func_addr(STRING_ID, 48))(s1, s2)
#define strtok(s1, s2) ((char *(*)(char *, const char *)) get_func_addr(STRING_ID, 49))(s1, s2)
#define strtok_r(s1, s2, s3) ((char *(*)(char *, const char *, char **)) get_func_addr(STRING_ID, 50))(s1, s2, s3)
#define strverscmp(s1, s2) ((int (*)(const char *, const char *)) get_func_addr(STRING_ID, 51))(s1, s2)

#endif
