/*****************************************************************************\
|   === sector.c : 2026 ===                                                   |
|                                                                             |
|    Filesystem v4 sector management functions                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

int fs_sector_get_unused(int disk, int count, sid_t *ret) {
    if (disk == 0) {
        static int next_free_sector_0 = 1;
        *ret = SID_FORMAT(disk, next_free_sector_0);
        next_free_sector_0 += count;
        return count;
    } else if (disk == 1) {
        static int next_free_sector_1 = 4000;
        *ret = SID_FORMAT(disk, next_free_sector_1);
        next_free_sector_1 += count;
        return count;
    } else {
        sys_error("fs_sector_get_unused: invalid disk %d", disk);
        return -1;
    }
}

int fs_sector_note_free(sid_t sid) {
    UNUSED(sid);
    sys_error("fs_sector_note_free: not implemented yet");
    return 0;
}
