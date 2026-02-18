/*****************************************************************************\
|   === md5.h : 2025 ===                                                      |
|                                                                             |
|    Header file for MD5 Algorithm implementation                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_MD5_H
#define _PROFAN_MD5_H

#include <stdio.h>

_BEGIN_C_FILE

// result must be a 16-byte buffer

int md5_stream(FILE *stream, void *result);
void *md5_buffer(const void *buffer, uint32_t len, void *result);

_END_C_FILE

#endif
