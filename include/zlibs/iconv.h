/*****************************************************************************\
|   === iconv.h : 2026 ===                                                    |
|                                                                             |
|    Implementation of the iconv.h header file from libC           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _ICONV_H
#define _ICONV_H

#include <profan/minimal.h>
#include <stddef.h>

_BEGIN_C_FILE

typedef void *iconv_t;

iconv_t iconv_open(const char *tocode, const char *fromcode);
int     iconv_close(iconv_t cd);

size_t iconv(iconv_t cd, char **inbuf, size_t *inbytesleft,
                char **outbuf, size_t *outbytesleft);

_END_C_FILE

#endif
