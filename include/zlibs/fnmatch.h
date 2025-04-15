/*****************************************************************************\
|   === fnmatch.h : 2025 ===                                                  |
|                                                                             |
|    Implementation of the fnmatch.h header file from libC         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _FNMATCH_H
#define _FNMATCH_H

#include <profan/minimal.h>

_BEGIN_C_FILE

#define FNM_NOMATCH     1       // Match failed
#define FNM_NOSYS       2       // Function not supported (unused)

#define FNM_NOESCAPE    0x01    // Disable backslash escaping
#define FNM_PATHNAME    0x02    // Slash must be matched by slash
#define FNM_PERIOD      0x04    // Period must be matched by period

#define FNM_LEADING_DIR 0x08    // Ignore /<tail> after Imatch
#define FNM_CASEFOLD    0x10    // Case insensitive search

#define FNM_IGNORECASE  FNM_CASEFOLD
#define FNM_FILE_NAME   FNM_PATHNAME

int fnmatch(const char *pattern, const char *string, int flags);

_END_C_FILE

#endif
