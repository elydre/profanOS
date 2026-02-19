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

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

#define interdisk_rw_offset(sid, data, size, offset, is_read) \
    (is_read ? interdisk_read_offset(sid, data, size, offset) : interdisk_write_offset(sid, data, size, offset))


static int fs_cnt_rw(sid_t head_sid, void *buf, uint32_t offset, uint32_t size, int is_read) {
    sid_t loca_sid;

    // check if sector is cnt header
    if (interdisk_read(head_sid, sector_data, SECTOR_SIZE) || sector_data[0] != SF_HEAD) {
        sys_warning("d%ds%d not cnt header", SID_DISK(head_sid), SID_SECTOR(head_sid));
        return 1;
    }

    // check if offset+size is valid
    if (offset + size > sector_data[1]) {
        sys_warning("cannot %s beyond cnt size", is_read ? "read" : "write");
        return 1;
    }

    // rw locator
    loca_sid = sector_data[SECTOR_SIZE / sizeof(uint32_t) - 1];

    if (loca_sid == SID_NULL) {
        sys_error("INTERNAL ERROR: head cnt has no locator");
        return 1;
    }

    int index = -offset;

    while (index < (int) size) {
        // load locator sector
        if (interdisk_read(loca_sid, sector_data, SECTOR_SIZE)) {
            sys_error("failed to read d%ds%d", SID_DISK(loca_sid), SID_SECTOR(loca_sid));
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

            if (SID_IS_NULL(core_sid)) {
                sys_error("INTERNAL ERROR: null core d%ds%d", SID_DISK(loca_sid), SID_SECTOR(loca_sid));
                return 1;
            }

            uint32_t rwsize = min(size - index, SECTOR_SIZE * core_count);

            // read / write cores
            if (interdisk_rw_offset(
                    core_sid,
                    buf + index,
                    rwsize,
                    index < 0 ? -index : 0,
                    is_read
            )) {
                sys_error("failed to %s core d%ds%d",
                        is_read ? "read" : "write",
                        SID_DISK(core_sid), SID_SECTOR(core_sid)
                );
                return 1;
            }
            index += rwsize;
        }

        loca_sid = sector_data[SECTOR_SIZE / sizeof(uint32_t) - 1];

        if (SID_IS_NULL(loca_sid) && index < (int) size) {
            sys_error("INTERNAL ERROR: next locator null before end");
            return 1;
        }
    }

    return 0;
}

int fs_cnt_read(sid_t head_sid, void *buf, uint32_t offset, uint32_t size) {
    return fs_cnt_rw(head_sid, buf, offset, size, 1);
}

int fs_cnt_write(sid_t head_sid, const void *buf, uint32_t offset, uint32_t size) {
    return fs_cnt_rw(head_sid, (void *) buf, offset, size, 0);
}
