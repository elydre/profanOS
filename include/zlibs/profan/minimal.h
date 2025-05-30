/*****************************************************************************\
|   === minimal.h : 2025 ===                                                  |
|                                                                             |
|    Minimal header file for C++ compatibility                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_MINIMAL_H
#define _PROFAN_MINIMAL_H

#ifdef __cplusplus
  #define _BEGIN_C_FILE extern "C" {
  #define _END_C_FILE }
#else
  #define _BEGIN_C_FILE
  #define _END_C_FILE
#endif

#endif
