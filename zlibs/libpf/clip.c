/*****************************************************************************\
|   === clip.c : 2024 ===                                                     |
|                                                                             |
|    Clipboard utilities functions of libpf                        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/filesys.h>
#include <profan/clip.h>
#include <stdlib.h>
#include <string.h>

static uint32_t file_sid = SID_NULL;

int clip_set_str(char *str) {
    if (file_sid == SID_NULL)
        file_sid = fu_path_to_sid(SID_ROOT, PROFAN_CLIP_PATH);
    if (!fu_is_file(file_sid))
        return 1;

    if (str == NULL)
        return fu_file_set_size(file_sid, 0) ? 0 : 1;

    uint32_t size = strlen(str);

    if (fu_file_set_size(file_sid, size))
        return 1;
    return fu_file_write(file_sid, str, 0, size) ? 0 : 1;
}

int clip_set_raw(void *data, uint32_t size) {
    if (file_sid == SID_NULL)
        file_sid = fu_path_to_sid(SID_ROOT, PROFAN_CLIP_PATH);
    if (!fu_is_file(file_sid))
        return 1;

    if (data == NULL)
        return fu_file_set_size(file_sid, 0) ? 0 : 1;

    if (fu_file_set_size(file_sid, size))
        return 1;
    return fu_file_write(file_sid, data, 0, size) ? 0 : 1;
}

char *clip_get_str(void) {
    if (file_sid == SID_NULL)
        file_sid = fu_path_to_sid(SID_ROOT, PROFAN_CLIP_PATH);
    if (!fu_is_file(file_sid))
        return NULL;
    uint32_t size = fu_file_get_size(file_sid);
    char *buf = malloc(size + 1);

    fu_file_read(file_sid, buf, 0, size);

    buf[size] = '\0';

    return buf;
}

void *clip_get_raw(uint32_t *size) {
    if (file_sid == SID_NULL)
        file_sid = fu_path_to_sid(SID_ROOT, PROFAN_CLIP_PATH);
    if (!fu_is_file(file_sid))
        return NULL;
    *size = fu_file_get_size(file_sid);
    void *buf = malloc(*size);

    fu_file_read(file_sid, buf, 0, *size);

    return buf;
}
