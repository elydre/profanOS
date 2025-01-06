/*****************************************************************************\
|   === wchar.c : 2025 ===                                                    |
|                                                                             |
|    Implementation of wchar functions from libC                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan.h>
#include <wchar.h>

wint_t btowc(int) {
    return (PROFAN_FNI, 0);
}

int fwprintf(FILE *, const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

int fwscanf(FILE *, const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

int iswalnum(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswalpha(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswcntrl(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswdigit(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswgraph(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswlower(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswprint(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswpunct(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswspace(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswupper(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswxdigit(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswctype(wint_t, wctype_t) {
    return (PROFAN_FNI, 0);
}

wint_t fgetwc(FILE *) {
    return (PROFAN_FNI, 0);
}

wchar_t *fgetws(wchar_t *, int, FILE *) {
    return (PROFAN_FNI, NULL);
}

wint_t fputwc(wchar_t, FILE *) {
    return (PROFAN_FNI, 0);
}

int fputws(const wchar_t *, FILE *) {
    return (PROFAN_FNI, 0);
}

wint_t getwc(FILE *) {
    return (PROFAN_FNI, 0);
}

wint_t getwchar(void) {
    return (PROFAN_FNI, 0);
}

int mbsinit(const mbstate_t *) {
    return (PROFAN_FNI, 0);
}

size_t mbrlen(const char *, size_t, mbstate_t *) {
    return (PROFAN_FNI, 0);
}

size_t mbrtowc(wchar_t *, const char *, size_t, mbstate_t *) {
    return (PROFAN_FNI, 0);
}

size_t mbsrtowcs(wchar_t *, const char **, size_t, mbstate_t *) {
    return (PROFAN_FNI, 0);
}

wint_t putwc(wchar_t, FILE *) {
    return (PROFAN_FNI, 0);
}

wint_t putwchar(wchar_t) {
    return (PROFAN_FNI, 0);
}

int swprintf(wchar_t *, size_t, const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

int swscanf(const wchar_t *, const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

wint_t towlower(wint_t) {
    return (PROFAN_FNI, 0);
}

wint_t towupper(wint_t) {
    return (PROFAN_FNI, 0);
}

wint_t ungetwc(wint_t, FILE *) {
    return (PROFAN_FNI, 0);
}

int vfwprintf(FILE *, const wchar_t *, va_list) {
    return (PROFAN_FNI, 0);
}

int vwprintf(const wchar_t *, va_list) {
    return (PROFAN_FNI, 0);
}

int vswprintf(wchar_t *, size_t, const wchar_t *, va_list) {
    return (PROFAN_FNI, 0);
}

size_t wcrtomb(char *, wchar_t, mbstate_t *) {
    return (PROFAN_FNI, 0);
}

wchar_t *wcscat(wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wcschr(const wchar_t *str, wchar_t c) {
    if (str == NULL)
        return NULL;

    size_t i = 0;
    while (str[i] != L'\0') {
        if (str[i] == c)
            return (wchar_t *) &str[i];
        i++;
    }

    if (c == L'\0')
        return (wchar_t *) &str[i];

    return NULL;
}

int wcscmp(const wchar_t *str1, const wchar_t *str2) {
    if (str1 == NULL || str2 == NULL)
        return (str1 == str2 ? 0 : (str1 ? 1 : -1));

    size_t i = 0;
    while (str1[i] != L'\0' && str2[i] != L'\0') {
        if (str1[i] != str2[i])
            return (int) (str1[i] - str2[i]);
        i++;
    }

    return (int)(str1[i] - str2[i]);
}

int wcscoll(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, 0);
}

wchar_t *wcscpy(wchar_t *str1, const wchar_t *str2) {
    if (str1 == NULL || str2 == NULL)
        return NULL;

    size_t i = 0;
    while (str2[i] != L'\0') {
        str1[i] = str2[i];
        i++;
    }

    str1[i] = L'\0';

    return str1;
}

size_t wcscspn(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, 0);
}

size_t wcsftime(wchar_t *, size_t, const wchar_t *, const struct tm *) {
    return (PROFAN_FNI, 0);
}

size_t wcslen(const wchar_t *str) {
    size_t len = 0;

    if (str == NULL)
        return 0;

    while (str[len] != L'\0')
        len++;

    return len;
}

size_t wcsnlen(const wchar_t *str, size_t maxlen) {
    size_t len = 0;

    if (str == NULL)
        return 0;

    while (len < maxlen && str[len] != L'\0')
        len++;

    return len;
}

wchar_t *wcsncat(wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, NULL);
}

int wcsncmp(const wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, 0);
}

wchar_t *wcsncpy(wchar_t *str1, const wchar_t *str2, size_t n) {
    if (str1 == NULL || str2 == NULL)
        return NULL;

    size_t i = 0;
    while (i < n && str2[i] != L'\0') {
        str1[i] = str2[i];
        i++;
    }

    if (i < n)
        str1[i] = L'\0';

    return str1;
}

wchar_t *wcspbrk(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wcsrchr(const wchar_t *str, wchar_t c) {
    if (str == NULL)
        return NULL;

    size_t i = wcslen(str);
    while (i > 0) {
        i--;
        if (str[i] == c)
            return (wchar_t *) &str[i];
    }

    return NULL;
}

size_t wcsrtombs(char *, const wchar_t **, size_t, mbstate_t *) {
    return (PROFAN_FNI, 0);
}

size_t wcsspn(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, 0);
}

wchar_t *wcsstr(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wcstok(wchar_t *, const wchar_t *, wchar_t **) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wcswcs(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, NULL);
}

int wcswidth(const wchar_t *, size_t) {
    return (PROFAN_FNI, 0);
}

size_t wcsxfrm(wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, 0);
}

int wctob(wint_t) {
    return (PROFAN_FNI, 0);
}

wctype_t wctype(const char *) {
    return (PROFAN_FNI, 0);
}

int wcwidth(wchar_t) {
    return (PROFAN_FNI, 0);
}

wchar_t *wmemchr(const wchar_t *, wchar_t, size_t) {
    return (PROFAN_FNI, NULL);
}

int wmemcmp(const wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, 0);
}

wchar_t *wmemcpy(wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wmemmove(wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wmemset(wchar_t *, wchar_t, size_t) {
    return (PROFAN_FNI, NULL);
}

int wprintf(const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

int wscanf(const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

double wcstod(const wchar_t *, wchar_t **) {
    return (PROFAN_FNI, 0);
}

float wcstof(const wchar_t *, wchar_t **) {
    return (PROFAN_FNI, 0);
}

long int wcstol(const wchar_t *, wchar_t **, int) {
    return (PROFAN_FNI, 0);
}

long long int wcstoll(const wchar_t *, wchar_t **, int) {
    return (PROFAN_FNI, 0);
}

unsigned long int wcstoul(const wchar_t *, wchar_t **, int) {
    return (PROFAN_FNI, 0);
}

unsigned long long int wcstoull(const wchar_t *, wchar_t **, int) {
    return (PROFAN_FNI, 0);
}
