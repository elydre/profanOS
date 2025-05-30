/*****************************************************************************\
|   === wctype.h : 2025 ===                                                   |
|                                                                             |
|    Implementation of the wctype.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _WCTYPE_H
#define _WCTYPE_H

#include <profan/minimal.h>
#include <wchar.h>

_BEGIN_C_FILE

#define WEOF ((wint_t) -1)

typedef unsigned long int wctrans_t;

int       iswalnum(wint_t);
int       iswalpha(wint_t);
int       iswblank(wint_t);
int       iswcntrl(wint_t);
int       iswdigit(wint_t);
int       iswgraph(wint_t);
int       iswlower(wint_t);
int       iswprint(wint_t);
int       iswpunct(wint_t);
int       iswspace(wint_t);
int       iswupper(wint_t);
int       iswxdigit(wint_t);
int       iswctype(wint_t, wctype_t);
wint_t    towctrans(wint_t, wctrans_t);
wint_t    towlower(wint_t);
wint_t    towupper(wint_t);
wctrans_t wctrans(const char *);
wctype_t  wctype(const char *);

_END_C_FILE

#endif
