/*****************************************************************************\
|   === wchar.h : 2025 ===                                                    |
|                                                                             |
|    Implementation of the wchar.h header file from libC           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _WCHAR_H
#define _WCHAR_H

#include <stddef.h>
#include <stdio.h>  // FILE
#include <time.h>   // struct tm

typedef unsigned int wint_t;
typedef unsigned long int wctype_t;

typedef struct {
    int __count;
    union {
        wint_t __wch;
        char __wchb[4];
    } __value;
} mbstate_t;

wint_t    btowc(int);
int       fwprintf(FILE *, const wchar_t *, ...);
int       fwscanf(FILE *, const wchar_t *, ...);
wint_t    fgetwc(FILE *);
wchar_t  *fgetws(wchar_t *, int, FILE *);
wint_t    fputwc(wchar_t, FILE *);
int       fputws(const wchar_t *, FILE *);
wint_t    getwc(FILE *);
wint_t    getwchar(void);
int       mbsinit(const mbstate_t *);
size_t    mbrlen(const char *, size_t, mbstate_t *);
size_t    mbrtowc(wchar_t *, const char *, size_t, mbstate_t *);
size_t    mbsrtowcs(wchar_t *, const char **, size_t, mbstate_t *);
wint_t    putwc(wchar_t, FILE *);
wint_t    putwchar(wchar_t);
int       swprintf(wchar_t *, size_t, const wchar_t *, ...);
int       swscanf(const wchar_t *, const wchar_t *, ...);
wint_t    ungetwc(wint_t, FILE *);
int       vfwprintf(FILE *, const wchar_t *, va_list);
int       vwprintf(const wchar_t *, va_list);
int       vswprintf(wchar_t *, size_t, const wchar_t *, va_list);
size_t    wcrtomb(char *, wchar_t, mbstate_t *);
wchar_t  *wcscat(wchar_t *, const wchar_t *);
wchar_t  *wcschr(const wchar_t *, wchar_t);
int       wcscmp(const wchar_t *, const wchar_t *);
int       wcscoll(const wchar_t *, const wchar_t *);
wchar_t  *wcscpy(wchar_t *, const wchar_t *);
size_t    wcscspn(const wchar_t *, const wchar_t *);
size_t    wcsftime(wchar_t *, size_t, const wchar_t *, const struct tm *);
size_t    wcslen(const wchar_t *);
size_t    wcsnlen(const wchar_t *, size_t);
wchar_t  *wcsncat(wchar_t *, const wchar_t *, size_t);
int       wcsncmp(const wchar_t *, const wchar_t *, size_t);
wchar_t  *wcsncpy(wchar_t *, const wchar_t *, size_t);
wchar_t  *wcspbrk(const wchar_t *, const wchar_t *);
wchar_t  *wcsrchr(const wchar_t *, wchar_t);
size_t    wcsrtombs(char *, const wchar_t **, size_t, mbstate_t *);
size_t    wcsspn(const wchar_t *, const wchar_t *);
wchar_t  *wcsstr(const wchar_t *, const wchar_t *);
wchar_t  *wcstok(wchar_t *, const wchar_t *, wchar_t **);
wchar_t  *wcswcs(const wchar_t *, const wchar_t *);
int       wcswidth(const wchar_t *, size_t);
size_t    wcsxfrm(wchar_t *, const wchar_t *, size_t);
int       wctob(wint_t);
int       wcwidth(wchar_t);
wchar_t  *wmemchr(const wchar_t *, wchar_t, size_t);
int       wmemcmp(const wchar_t *, const wchar_t *, size_t);
wchar_t  *wmemcpy(wchar_t *, const wchar_t *, size_t);
wchar_t  *wmemmove(wchar_t *, const wchar_t *, size_t);
wchar_t  *wmemset(wchar_t *, wchar_t, size_t);
int       wprintf(const wchar_t *, ...);
int       wscanf(const wchar_t *, ...);

double             wcstod(const wchar_t *nptr, wchar_t **endptr);
float              wcstof(const wchar_t *nptr, wchar_t **endptr);
long               wcstol(const wchar_t *nptr, wchar_t **endptr, int base);
long long          wcstoll(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long      wcstoul(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long long wcstoull(const wchar_t *nptr, wchar_t **endptr, int base);

#endif
