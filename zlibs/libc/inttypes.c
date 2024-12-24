/*****************************************************************************\
|   === inttypes.c : 2024 ===                                                 |
|                                                                             |
|    Implementation of inttypes functions from libC                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <inttypes.h>
#include <stdlib.h>

intmax_t imaxabs(intmax_t n) {
    return n < 0 ? -n : n;
}

imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom) {
    imaxdiv_t result;
    result.quot = numer / denom;
    result.rem  = numer % denom;
    return result;
}

intmax_t strtoimax(const char *nptr, char **endptr, int base) {
    return strtoll(nptr, endptr, base);
}

uintmax_t strtoumax(const char *nptr, char **endptr, int base) {
    return strtoull(nptr, endptr, base);
}

intmax_t wcstoimax(const wchar_t *nptr, wchar_t **endptr, int base) {
    return wcstoll(nptr, endptr, base);
}

uintmax_t wcstoumax(const wchar_t *nptr, wchar_t **endptr, int base) {
    return wcstoull(nptr, endptr, base);
}
