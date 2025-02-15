/*****************************************************************************\
|   === wctype.c : 2025 ===                                                   |
|                                                                             |
|    Implementation of wctype functions from libC                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <wctype.h>
#include <profan.h>

int iswalnum(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswalpha(wint_t) {
    return (PROFAN_FNI, 0);
}

int iswblank(wint_t) {
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

int iswprint(wint_t wc) {
    if (wc < 0xffU)
        return ((wc + 1) & 0x7f) >= 0x21;
    if (wc < 0x2028U || wc - 0x202aU < 0xd800 - 0x202a || wc - 0xe000U < 0xfff9 - 0xe000)
        return 1;
    if (wc - 0xfffcU > 0x10ffff - 0xfffc || (wc & 0xfffe) == 0xfffe)
        return 0;
    return 1;
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

wint_t towctrans(wint_t, wctrans_t) {
    return (PROFAN_FNI, 0);
}

wint_t towlower(wint_t) {
    return (PROFAN_FNI, 0);
}

wint_t towupper(wint_t) {
    return (PROFAN_FNI, 0);
}

wctrans_t wctrans(const char *) {
    return (PROFAN_FNI, 0);
}

wctype_t wctype(const char *) {
    return (PROFAN_FNI, 0);
}
