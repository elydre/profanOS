/*****************************************************************************\
|   === microwave.c : 2024 ===                                                |
|                                                                             |
|    A clipboard                                                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/libmmq.h>
#include <profan/filesys.h>

typedef uint16_t u16;
typedef uint8_t u8;
typedef uint32_t u32;

typedef struct {
    short type;
    union {
        char *str;
    }val;
} element_t;

enum {
    CLIP_STR,
    CLIP_BYTE,
};

u32 clip_len = 0;
element_t *clip = NULL;

static int clip_file(void *buffer, u32 offset, u32 size, u8 is_read) {
	// TODO! : implement that please
	return 0;
}

int init() {
	fu_fctf_create(0, "/dev/clip", &clip_file);
    return 0;
}

static char *clip_strdup(char *str) {
    int len = strlen(str);
    char *res = malloc_ask(len + 1);
    strcpy(res, str);
    return res;
}

void clip_push_str(char *str) {
    clip_len++;
    clip = realloc_ask(clip, sizeof(element_t) * (clip_len));
    clip[clip_len - 1].type = CLIP_STR;
    clip[clip_len - 1].val.str = clip_strdup(str);
}

element_t *clip_get_last() {
    if (clip_len == 0)
        return (NULL);
    return &(clip[clip_len - 1]);
}

void clip_pop() {
    if (clip_len == 0)
        return ;
    if (clip[clip_len - 1].type == CLIP_STR)
        free(clip[clip_len - 1].val.str);
    clip_len--;
    clip = realloc_ask(clip, sizeof(element_t) * (clip_len));

