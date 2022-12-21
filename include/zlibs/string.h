#ifndef STDLIB_ID
#define STDLIB_ID 1008

#include <type.h>

#define get_func_addr ((int (*)(int, int)) *(int *) 0x199994)

/*
int main();
char *basename(const char *path);
void bcopy(const void *s2, void *s1, size_t n);
*/

#define basename(path) ((char *(*)(const char *)) get_func_addr(STDLIB_ID, 3))(path)
#define bcopy(s2, s1, n) ((void (*)(const void *, void *, size_t)) get_func_addr(STDLIB_ID, 4))(s2, s1, n)
#define bzero(s, n) ((void (*)(void *, size_t)) get_func_addr(STDLIB_ID, 5))(s, n)
#define dirname(path) ((char *(*)(const char *)) get_func_addr(STDLIB_ID, 6))(path)
#define ffs(i) ((int (*)(int)) get_func_addr(STDLIB_ID, 7))(i)
#define ffsll(i) ((int (*)(long long)) get_func_addr(STDLIB_ID, 8))(i)
#define memccpy(s1, s2, c, n) ((void *(*)(void *, const void *, int, size_t)) get_func_addr(STDLIB_ID, 9))(s1, s2, c, n)
#define memchr(s, c, n) ((wchar_t *(*)(const wchar_t *, wchar_t, size_t)) get_func_addr(STDLIB_ID, 10))(s, c, n)
#define memcmp(s1, s2, n) ((int (*)(const wchar_t *, const wchar_t *, size_t)) get_func_addr(STDLIB_ID, 11))(s1, s2, n)
#define memcpy(s1, s2, n) ((void *(*)(void *, const void *, size_t)) get_func_addr(STDLIB_ID, 12))(s1, s2, n)
#define memmem(haystack, hlen, needle, nlen) ((void *(*)(const void *, size_t, const void *, size_t)) get_func_addr(STDLIB_ID, 13))(haystack, hlen, needle, nlen)
#define memmove(s1, s2, n) ((void *(*)(void *, const void *, size_t)) get_func_addr(STDLIB_ID, 14))(s1, s2, n)
#define mempcpy(s1, s2, n) ((void *(*)(void *, const void *, size_t)) get_func_addr(STDLIB_ID, 15))(s1, s2, n)
#define memrchr(s, c, n) ((void *(*)(const void *, int, size_t)) get_func_addr(STDLIB_ID, 16))(s, c, n)
#define memset(s, c, n) ((void *(*)(void *, int, size_t)) get_func_addr(STDLIB_ID, 17))(s, c, n)

#endif
