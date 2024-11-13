/*****************************************************************************\
|   === microwave.h : 2024 ===                                                |
|                                                                             |
|    A clipboard                                                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef MICROWAVE_ID_H
#define MICROWAVE_ID_H 1008

#include <stdint.h>

typedef uint16_t u16;
typedef uint8_t u8;
typedef uint32_t u32;

typedef struct {
    short type;
    union {
        u8 *str;
        struct {
            u8 *b;
            u32 len;
        } bytes;
    }val;
} element_t;

enum {
    CLIP_STR,
    CLIP_BYTE,
};

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define clip_push_str ((void (*)(char *)) get_func_addr(MICROWAVE_ID_H, 2))
#define clip_get_last ((element_t * (*)(void)) get_func_addr(MICROWAVE_ID_H, 3))
#define clip_pop    ((void (*)(void)) get_func_addr(MICROWAVE_ID_H, 4))

#endif
