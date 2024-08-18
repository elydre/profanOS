/*****************************************************************************\
|   === ctr_rw.c : 2024 ===                                                   |
|                                                                             |
|    Kernel container read/write functions                         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

int fs_cnt_rw_core(filesys_t *filesys, sid_t core_sid, uint8_t *buf, uint32_t offset,
        uint32_t size, int is_read, uint8_t *alloc_buf) {
    vdisk_t *vdisk;

    // check if offset is valid
    if (offset >= BYTE_IN_CORE) {
        return -1;
    }

    vdisk = fs_get_vdisk(filesys, core_sid.device);

    if (vdisk == NULL) {
        return -1;
    }

    // check if sector is used
    if (!vdisk_is_sector_used(vdisk, core_sid)) {
        return -1;
    }

    // check if sector is core
    // data = vdisk_load_sector(vdisk, core_sid);
    vdisk_read_sector(vdisk, core_sid, alloc_buf);

    if (alloc_buf[0] != ST_CONT || alloc_buf[1] != SF_CORE) {
        return -1;
    }

    size = min(size, BYTE_IN_CORE - offset);
    offset += 2;

    if (!is_read) {
        mem_copy(alloc_buf + offset, buf, size);
        vdisk_write_sector(vdisk, core_sid, alloc_buf);
    } else {
        mem_copy(buf, alloc_buf + offset, size);
    }

    return size + offset - 2;
}

int fs_cnt_rw_loca(filesys_t *filesys, sid_t loca_sid, uint8_t *buf,
        uint32_t offset, int size, int is_read) {
    sid_t next_loca_sid;
    vdisk_t *vdisk;

    uint8_t *alloc_buf = malloc(FS_SECTOR_SIZE);
    uint8_t *data = malloc(FS_SECTOR_SIZE);
    int tmp;

    vdisk = fs_get_vdisk(filesys, loca_sid.device);

    if (vdisk == NULL) {
        return 1;
    }

    int index = -offset;
    while (index < size) {
        // check if sector is used
        if (!vdisk_is_sector_used(vdisk, loca_sid)) {
            sys_error("[cnt_rw] locator not used");
            free(alloc_buf);
            free(data);
            return 1;
        }

        // check if sector is locator
        // data = vdisk_load_sector(vdisk, loca_sid);
        vdisk_read_sector(vdisk, loca_sid, data);

        if (data[0] != ST_CONT || data[1] != SF_LOCA) {
            free(alloc_buf);
            free(data);
            return 1;
        }

        if (index + LINKS_IN_LOCA * BYTE_IN_CORE < 0) {
            index += LINKS_IN_LOCA * BYTE_IN_CORE;
        } else for (int i = 0; i < LINKS_IN_LOCA; i++) {
            if (index >= size) {
                free(alloc_buf);
                free(data);
                return 0;
            }
            if (index + BYTE_IN_CORE <= 0) {
                index += BYTE_IN_CORE;
                continue;
            }
            sid_t core_sid = *((sid_t *) (data + (i + 1) * sizeof(sid_t)));
            if (IS_NULL_SID(core_sid)) {
                sys_error("no more core, but still %d bytes to %s\n", size - max(index, 0), is_read ? "read" : "write");
                free(alloc_buf);
                free(data);
                return 1;
            }
            tmp = fs_cnt_rw_core(filesys, core_sid, buf + max(index, 0), max(0, -index),
                    size - max(index, 0), is_read, alloc_buf);
            if (tmp == -1) {
                sys_error("failed to %s core d%ds%d\n", is_read ? "read" : "write", core_sid.device, core_sid.sector);
                free(alloc_buf);
                free(data);
                return 1;
            }
            index += tmp;
        }

        next_loca_sid = *((sid_t *) (data + LAST_SID_OFFSET));
        if (IS_NULL_SID(next_loca_sid) && index < size) {
            sys_error("no more locator after d%ds%d, but still %d bytes to %s\n",
                    loca_sid.device,
                    loca_sid.sector,
                    size - index,
                    is_read ? "read" : "write"
            );
            free(alloc_buf);
            free(data);
            return 1;
        }
        loca_sid = next_loca_sid;
    }
    free(alloc_buf);
    free(data);
    return 0;
}

int fs_cnt_rw(filesys_t *filesys, sid_t head_sid, void *buf, uint32_t offset, uint32_t size, int is_read) {
    vdisk_t *vdisk;
    uint8_t *data;
    sid_t loca_sid;

    vdisk = fs_get_vdisk(filesys, head_sid.device);

    if (vdisk == NULL || !vdisk_is_sector_used(vdisk, head_sid)) {
        sys_warning("[cnt_rw] Invalid sector id");
        return 1;
    }

    // check if sector is cnt header
    data = vdisk_load_sector(vdisk, head_sid);

    if (data[0] != ST_CONT || data[1] != SF_HEAD) {
        sys_warning("[cnt_rw] Sector is not cnt header");
        vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
        return 1;
    }

    // check if offset+size is valid
    if (offset + size > *((uint32_t *) (data + 2 + META_MAXLEN))) {
        sys_warning("[cnt_rw] cannot %s beyond cnt size (%d requested, %d max)",
                is_read ? "read" : "write",
                offset + size,
                *((uint32_t *) (data + 2 + META_MAXLEN))
        );
        vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
        return 1;
    }

    // rw locator
    loca_sid = *((sid_t *) (data + LAST_SID_OFFSET));
    if (loca_sid.device != 0 || loca_sid.sector != 0) {
        if (fs_cnt_rw_loca(filesys, loca_sid, (uint8_t *) buf, offset, (int) size, is_read)) {
            vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
            return 1;
        }
    }

    vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
    return 0;
}

int fs_cnt_read(filesys_t *filesys, sid_t head_sid, void *buf, uint32_t offset, uint32_t size) {
    return fs_cnt_rw(filesys, head_sid, buf, offset, size, 1);
}

int fs_cnt_write(filesys_t *filesys, sid_t head_sid, void *buf, uint32_t offset, uint32_t size) {
    return fs_cnt_rw(filesys, head_sid, buf, offset, size, 0);
}
