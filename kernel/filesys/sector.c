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

int fs_sector_get_unused(int disk, uint32_t count, sid_t *ret) {
    sid_t free_map = SID_FORMAT(disk, 0);

    uint32_t last[2];

    int size = fs_cnt_get_size(free_map);

    if (size < (int) sizeof(last)) {
        sys_error("[fs_sector_get_unused] Disk %d free map corrupted", disk);
        return -1;
    }

    if (fs_cnt_read(free_map, last, size - sizeof(last), sizeof(last))) {
        sys_error("[fs_sector_get_unused] Disk %d free map read error", disk);
        return -1;
    }

    uint32_t first_sector = last[0];
    uint32_t free_count   = last[1];

    if (free_count == (uint32_t) -1) {
        // get the disk size
    }

    if (free_count <= count) {
        if (fs_cnt_set_size(free_map, size - sizeof(last))) {
            sys_error("[fs_sector_get_unused] Disk %d free map update error", disk);
            return -1;
        }
        *ret = SID_FORMAT(disk, first_sector);
        return count;
    }

    last[0] += count;
    last[1] -= count;

    if (fs_cnt_write(free_map, last, size - sizeof(last), sizeof(last))) {
        sys_error("[fs_sector_get_unused] Disk %d free map update error", disk);
        return -1;
    }

    *ret = SID_FORMAT(disk, first_sector);
    return count;
}

int fs_sector_note_free(sid_t sid, uint32_t count) {
}
