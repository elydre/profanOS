/*****************************************************************************\
|   === ctr_del.c : 2026 ===                                                  |
|                                                                             |
|    Kernel container deletion functions                           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

int fs_cnt_delete(sid_t head_sid) {
    if (fs_cnt_set_size(head_sid, 0))
        return 1;

    if (interdisk_read(head_sid, sector_data, SECTOR_SIZE) || sector_data[0] != SF_HEAD)
        return 1;

    sid_t loca_sid = sector_data[SECTOR_SIZE / sizeof(uint32_t) - 1];

    if (!SID_IS_NULL(loca_sid) && fs_sector_note_free(SID_RESTORE_DISK(loca_sid, head_sid)))
        return 1;

    return fs_sector_note_free(head_sid);
}
