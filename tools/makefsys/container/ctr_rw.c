/*****************************************************************************\
|   === ctr_rw.c : 2025 ===                                                   |
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

#define vdisk_rw(data, size, offset, is_read) \
    (is_read ? vdisk_read(data, size, offset) : vdisk_write(data, size, offset))


int fs_cnt_rw(sid_t head_sid, void *buf, uint32_t offset, uint32_t size, int is_read) {

    sid_t loca_sid;

    // check if sector is cnt header

    if (vdisk_read(sector_data, SECTOR_SIZE, SID_SECTOR(head_sid) * SECTOR_SIZE) || sector_data[0] != SF_HEAD) {
        printf("d%ds%d not cnt header\n", SID_DISK(head_sid), SID_SECTOR(head_sid));
        return 1;
    }

    // check if offset+size is valid
    if (offset + size > sector_data[1]) {
        printf("cannot %s beyond cnt size\n", is_read ? "read" : "write");
        return 1;
    }

    // rw locator
    loca_sid = sector_data[SECTOR_SIZE / sizeof(uint32_t) - 1];

    if (loca_sid == SID_NULL) {
        printf("INTERNAL ERROR: head cnt has no locator\n");
        return 1;
    }

    int index = -offset;

    while (index < size) {
        // load locator sector
        if (vdisk_read(sector_data, SECTOR_SIZE, loca_sid * SECTOR_SIZE)) {
            printf("failed to read d%ds%d\n", SID_DISK(loca_sid), SID_SECTOR(loca_sid));
            return 1;
        }

        for (int i = 0; i < LINKS_IN_LOCA; i++) {
            if (index >= (int) size)
                return 0;

            sid_t    core_sid   = sector_data[i * 2];
            uint32_t core_count = sector_data[i * 2 + 1];

            if (index + (SECTOR_SIZE * core_count) <= 0) {
                index += SECTOR_SIZE * core_count;
                continue;
            }

            if (IS_SID_NULL(core_sid)) {
                printf("INTERNAL ERROR: null core d%ds%d\n", SID_DISK(loca_sid), SID_SECTOR(loca_sid));
                return 1;
            }

            uint32_t rwsize = min(size - index, SECTOR_SIZE * core_count);

            // read / write cores
            if (vdisk_rw(
                    buf + index,
                    rwsize,
                    (core_sid * SECTOR_SIZE) + (index < 0 ? -index : 0),
                    is_read
            )) {
                printf("failed to %s core d%ds%d\n",
                        is_read ? "read" : "write",
                        SID_DISK(core_sid), SID_SECTOR(core_sid)
                );
                return 1;
            }
            index += rwsize;
        }

        loca_sid = sector_data[SECTOR_SIZE / sizeof(uint32_t) - 1];

        if (IS_SID_NULL(loca_sid) && index < (int) size) {
            printf("INTERNAL ERROR: next locator null before end\n");
            return 1;
        }
    }

    return 0;
}
