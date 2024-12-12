/*****************************************************************************\
|   === clip.h : 2024 ===                                                     |
|                                                                             |
|    Header file for clipboard utilities functions of libpf        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_CLIP_H
#define _PROFAN_CLIP_H

#define PROFAN_CLIP_PATH "/dev/clip"

int   clip_set_str(char *str);
int   clip_set_raw(void *data, uint32_t size);

char *clip_get_str(void);
void *clip_get_raw(uint32_t *size);

#endif
