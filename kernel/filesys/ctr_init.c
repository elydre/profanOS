/*****************************************************************************\
|   === ctr_init.c : 2025 ===                                                 |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

sid_t fs_cnt_init(uint32_t device_id, const char *meta) {
    sid_t main_sid;
    sid_t loca_sid;

    // get new sector for header
    if (fs_sector_get_unused(device_id, 1, &main_sid) != 1) {
        sys_warning("no more sectors in d%d\n", device_id);
        return SID_NULL;
    }

    // get new sector for locator
    if (fs_sector_get_unused(device_id, 1, &loca_sid) != 1) {
        sys_warning("no more sectors in d%d\n", SID_DISK(main_sid));
        fs_sector_note_free(main_sid);
        return SID_NULL;
    }

    // init header
    mem_set(sector_data, 0, SECTOR_SIZE);
    sector_data[0] = SF_HEAD;
    sector_data[1] = 0; // size

    mem_copy(sector_data + 2, meta, min(META_MAXLEN, str_len(meta) + 1));

    sector_data[SECTOR_SIZE / sizeof(uint32_t) - 1] = loca_sid;

    if (vdisk_write(sector_data, SECTOR_SIZE, SID_SECTOR(main_sid) * SECTOR_SIZE)) {
        sys_error("failed to write d%ds%d\n", SID_DISK(main_sid), SID_SECTOR(main_sid));
        goto init_err;
    }

    // init locator
    mem_set(sector_data, 0, SECTOR_SIZE);
    sector_data[SECTOR_SIZE / sizeof(uint32_t) - 1] = SID_NULL;

    if (vdisk_write(sector_data, SECTOR_SIZE, SID_SECTOR(loca_sid) * SECTOR_SIZE)) {
        sys_error("failed to write d%ds%d\n", SID_DISK(loca_sid), SID_SECTOR(loca_sid));
        goto init_err;
    }

    // return main sid
    return main_sid;

    init_err:
    fs_sector_note_free(main_sid);
    fs_sector_note_free(loca_sid);
    return SID_NULL;
}


int fs_cnt_meta(sid_t sid, char *meta, int buffer_size, int replace) {
    if (vdisk_read(sector_data, SECTOR_SIZE, SID_SECTOR(sid) * SECTOR_SIZE) || sector_data[0] != SF_HEAD) {
        sys_warning("not a cnt header d%ds%d\n", SID_DISK(sid), SID_SECTOR(sid));
        return 1;
    }

    if (!replace) {
        mem_copy(meta, sector_data + 2, min(buffer_size, META_MAXLEN));
        return 0;
    }

    mem_copy(sector_data + 2, meta, min(META_MAXLEN, buffer_size > 0 ? buffer_size : str_len(meta) + 1));

    if (vdisk_write(sector_data, SECTOR_SIZE, SID_SECTOR(sid) * SECTOR_SIZE)) {
        sys_error("failed to write d%ds%d\n", SID_DISK(sid), SID_SECTOR(sid));
        return 1;
    }

    return 0;
}
