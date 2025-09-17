/*****************************************************************************\
|   === ctr_size.c : 2025 ===                                                 |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../butterfly.h"

uint32_t sector_data[SECTOR_SIZE / sizeof(uint32_t)];

int fs_cnt_shrink_size(sid_t loca_sid, uint32_t to_shrink) {

    // check if sector is cnt locator

    if (vdisk_read(sector_data, SECTOR_SIZE, loca_sid * SECTOR_SIZE)) {
        printf("failed to read d%ds%d\n", SID_DISK(loca_sid), SID_SECTOR(loca_sid));
        return -1;
    }

    // check if sector linked to another locator
    sid_t *next_sid = &sector_data[SECTOR_SIZE / sizeof(uint32_t) - 1];

    if (!IS_SID_NULL(*next_sid)) {
        int ret = fs_cnt_shrink_size(*next_sid, to_shrink);
        if (ret <= 0) {
            return ret == 0 ? -2 : ret;
        }
        *next_sid = SID_NULL;
        to_shrink = ret;
    }

    // remove core sectors
    for (int i = LINKS_IN_LOCA - 1; i >= 0; i--) {
        if (to_shrink == 0)
            break;

        sid_t    core_sid   = sector_data[i * 2];
        uint32_t core_count = sector_data[i * 2 + 1];

        if (IS_SID_NULL(core_sid)) {
            continue;
        }

        for (int j = core_count - 1; j >= 0; j--) {
            if (to_shrink == 0)
                break;

            fs_sector_note_free(core_sid + j);
            core_count -= 1;
            to_shrink -= 1;
        }

        if (core_count == 0)
            sector_data[i * 2] = SID_NULL;
        sector_data[i * 2 + 1] = core_count;
    }

    // remove locator if empty
    if (to_shrink) {
        fs_sector_note_free(loca_sid);
    } else if (vdisk_write(sector_data, SECTOR_SIZE, loca_sid * SECTOR_SIZE)) {
        printf("failed to write d%ds%d\n", SID_DISK(loca_sid), SID_SECTOR(loca_sid));
        return -1;
    }

    return to_shrink;
}

int fs_cnt_grow_size(uint32_t loca_sid, uint32_t to_grow) {
    // jump to last locator
    do {
        if (vdisk_read(sector_data, SECTOR_SIZE, loca_sid * SECTOR_SIZE)) {
            printf("failed to read d%ds%d\n", SID_DISK(loca_sid), SID_SECTOR(loca_sid));
            return -1;
        }

        uint32_t next_sid = sector_data[SECTOR_SIZE / sizeof(uint32_t) - 1];

        if (IS_SID_NULL(next_sid)) {
            break;
        }

        loca_sid = next_sid;
    } while (1);

    // TODO

    return 0;
}

int fs_cnt_set_size(uint32_t head_sid, uint32_t size) {

    if (vdisk_read(sector_data, SECTOR_SIZE, head_sid * SECTOR_SIZE) || sector_data[0] != SF_HEAD) {
        printf("not a cnt header d%ds%d\n", SID_DISK(head_sid), SID_SECTOR(head_sid));
        return 1;
    }

    uint32_t old_count = (sector_data[1] / SECTOR_SIZE) + (sector_data[1] % SECTOR_SIZE ? 1 : 0);
    uint32_t new_count = size / SECTOR_SIZE + (size != 0);

    uint32_t loca_sid = sector_data[SECTOR_SIZE / sizeof(uint32_t) - 1];

    if (old_count < new_count) {
        // grow cnt
        if (fs_cnt_grow_size(loca_sid, new_count - old_count)) {
            return 1;
        }
    } else if (old_count > new_count) {
        // shrink cnt
        int ret = fs_cnt_shrink_size(loca_sid, old_count - new_count);
    
        if (ret == -2)  // TODO check if this can be improved
            ret = 0;
    
        if (ret) {
            printf("[cnt_set_size] Could not shrink cnt\n");
            return 1;
        }
    }

    sector_data[1] = size;

    if (vdisk_write(sector_data, SECTOR_SIZE, head_sid * SECTOR_SIZE)) {
        printf("failed to write d%ds%d\n", SID_DISK(head_sid), SID_SECTOR(head_sid));
        return 1;
    }

    return 0;
}

uint32_t fs_cnt_get_size(uint32_t head_sid) {
    if (vdisk_read(sector_data, SECTOR_SIZE, head_sid * SECTOR_SIZE) || sector_data[0] != SF_HEAD) {
        printf("not a cnt header d%ds%d\n", SID_DISK(head_sid), SID_SECTOR(head_sid));
        return UINT32_MAX;
    }

    return sector_data[1];
}
