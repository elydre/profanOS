/*****************************************************************************\
|   === stdalign.h : 2024 ===                                                 |
|                                                                             |
|    Implementation of the stdalign.h header file from libC        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original header - mintsuki/freestanding-headers               `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _STDALIGN_H
#define _STDALIGN_H

#ifndef __cplusplus

#if defined(__STDC_VERSION__) && __STDC_VERSION__ > 201710L
  // These do not need to be defined for C23+
#else
  #undef alignas
  #define alignas _Alignas

  #undef alignof
  #define alignof _Alignof

  #undef __alignof_is_defined
  #define __alignof_is_defined 1

  #undef __alignas_is_defined
  #define __alignas_is_defined 1
#endif
#endif

#endif
