/*****************************************************************************\
|   === iso646.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the iso646.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original header - mintsuki/freestanding-headers               `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef __FSTD_HDRS_ISO646_H
#define __FSTD_HDRS_ISO646_H 1

#ifndef __cplusplus

#undef and
#define and &&
#undef and_eq
#define and_eq &=
#undef bitand
#define bitand &
#undef bitor
#define bitor |
#undef compl
#define compl ~
#undef not
#define not !
#undef not_eq
#define not_eq !=
#undef or
#define or ||
#undef or_eq
#define or_eq |=
#undef xor
#define xor ^
#undef xor_eq
#define xor_eq ^=

#endif

#endif
