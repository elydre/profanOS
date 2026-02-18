/*****************************************************************************\
|   === stdbool.h : 2024 ===                                                  |
|                                                                             |
|    Implementation of the stdbool.h header file from libC         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original header - mintsuki/freestanding-headers               `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _STDBOOL_H
#define _STDBOOL_H

#ifndef __cplusplus
#if defined(__STDC_VERSION__) && __STDC_VERSION__ > 201710L
    // These do not need to be defined for C23+
#else
  #undef  bool
  #define bool _Bool

  #undef  true
  #define true 1

  #undef  false
  #define false 0
#endif
#endif

#undef  __bool_true_false_are_defined
#define __bool_true_false_are_defined 1

#endif
