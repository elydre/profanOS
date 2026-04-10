/*****************************************************************************\
|   === iconv.c : 2026 ===                                                    |
|                                                                             |
|    Implementation of iconv functions from libC                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan.h>
#include <iconv.h>

iconv_t iconv_open(const char *tocode, const char *fromcode) {
    return (PROFAN_FNI, (iconv_t) -1);
}

size_t iconv(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft) {
    return (PROFAN_FNI, (size_t) -1);
}

int iconv_close(iconv_t cd) {
    return (PROFAN_FNI, -1);
}
