/*****************************************************************************\
|   === ctr_size.c : 2024 ===                                                 |
|                                                                             |
|    Kernel container size functions                               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

int fs_cnt_shrink_size(filesys_t *filesys, uint32_t loca_sid, uint32_t to_shrink) {
    vdisk_t *vdisk;
    uint8_t *data;

    vdisk = fs_get_vdisk(filesys, SID_DISK(loca_sid));

    if (vdisk == NULL || !vdisk_is_sector_used(vdisk, loca_sid)) {
        sys_error("[cnt_shrink_size] Invalid sector id");
        return -1;
    }

    // check if sector is cnt locator
    data = vdisk_load_sector(vdisk, loca_sid);

    if (data[0] != SF_LOCA) {
        sys_error("[cnt_shrink_size] Sector is not container locator");
        free(data);
        return -1;
    }

    // check if sector linked to another locator
    uint32_t next_sid;
    mem_copy(&next_sid, data + LAST_SID_OFFSET, sizeof(uint32_t));

    if (!IS_SID_NULL(next_sid)) {
        int ret = fs_cnt_shrink_size(filesys, next_sid, to_shrink);
        if (ret <= 0) {
            vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
            return ret == 0 ? -2 : ret;
        }
        mem_set(data + LAST_SID_OFFSET, 0, sizeof(uint32_t));
        to_shrink = ret;
    }

    // remove core sectors
    for (uint32_t byte = LAST_SID_OFFSET - sizeof(uint32_t); to_shrink && byte > 0; byte -= sizeof(uint32_t)) {
        uint32_t core_sid;
        mem_copy(&core_sid, data + byte, sizeof(uint32_t));
        if (core_sid == 0)
            continue;
        vdisk_note_sector_unused(vdisk, core_sid);
        mem_set(data + byte, 0, sizeof(uint32_t));
        to_shrink--;
    }

    // remove locator if empty
    if (to_shrink) {
        vdisk_note_sector_unused(vdisk, loca_sid);
    }

    vdisk_unload_sector(vdisk, loca_sid, data, SAVE);
    return to_shrink;
}

int fs_cnt_grow_size(filesys_t *filesys, uint32_t loca_sid, uint32_t to_grow) {
    vdisk_t *vdisk;
    uint8_t *data;

    vdisk = fs_get_vdisk(filesys, SID_DISK(loca_sid));

    if (vdisk == NULL) {
        sys_error("[cnt_grow_size] vdisk not found");
        return -1;
    }

    uint32_t next_sid;

    do {
        // check if sector is used
        if (!vdisk_is_sector_used(vdisk, loca_sid)) {
            sys_error("[cnt_grow_size] Invalid sector id");
            return -1;
        }

        // check if sector is cnt locator
        data = vdisk_load_sector(vdisk, loca_sid);

        if (data[0] != SF_LOCA) {
            sys_error("[cnt_grow_size] Sector is not container locator");
            free(data);
            return -1;
        }

        mem_copy(&next_sid, data + LAST_SID_OFFSET, sizeof(uint32_t));
        vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);

        if (IS_SID_NULL(next_sid)) {
            break;
        }

        loca_sid = next_sid;
    } while (1);

    uint32_t core_count = 0;
    uint32_t new_loca_sid;
    uint32_t core_sid;

    data = vdisk_load_sector(vdisk, loca_sid);

    // check if loca is full
    for (uint32_t byte = sizeof(uint32_t); byte < LAST_SID_OFFSET; byte += sizeof(uint32_t)) {
        mem_copy(&core_sid, data + byte, sizeof(uint32_t));
        if (IS_SID_NULL(core_sid)) break;
        core_count += 1;
    }

    // fill loca
    while (core_count < LINKS_IN_LOCA && to_grow > 0) {
        core_sid = SID_FORMAT(SID_DISK(loca_sid), vdisk_get_unused_sector(vdisk));
        if (IS_SID_NULL(core_sid)) {
            sys_error("[cnt_grow_size] No more free sectors");
            vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
            return -1;
        }
        if (fs_cnt_init_core_in_sector(vdisk, core_sid) == -1) {
            sys_error("[cnt_grow_size] Could not init core");
            vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
            return -1;
        }
        core_count += 1;
        mem_copy(data + core_count * sizeof(uint32_t), &core_sid, sizeof(uint32_t));
        to_grow -= 1;
    }

    if (!to_grow) {
        vdisk_unload_sector(vdisk, loca_sid, data, SAVE);
        return 0;
    }

    while (to_grow > 0) {
        // create new locator

        new_loca_sid = SID_FORMAT(SID_DISK(loca_sid), vdisk_get_unused_sector(vdisk));

        if (IS_SID_NULL(new_loca_sid)) {
            sys_error("[cnt_grow_size] No more free sectors");
            vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
            return -1;
        }

        if (fs_cnt_init_loca_in_sector(vdisk, new_loca_sid) == -1) {
            sys_error("[cnt_grow_size] Could not init locator");
            vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
            return -1;
        }

        mem_copy(data + LAST_SID_OFFSET, &new_loca_sid, sizeof(uint32_t));
        vdisk_unload_sector(vdisk, loca_sid, data, SAVE);

        loca_sid = new_loca_sid;

        data = vdisk_load_sector(vdisk, loca_sid);

        // fill loca
        core_count = 0;
        while (core_count < LINKS_IN_LOCA && to_grow > 0) {
            core_sid = SID_FORMAT(SID_DISK(loca_sid), vdisk_get_unused_sector(vdisk));
            if (IS_SID_NULL(core_sid)) {
                sys_error("[cnt_grow_size] No more free sectors");
                vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
                return -1;
            }
            if (fs_cnt_init_core_in_sector(vdisk, core_sid) == -1) {
                sys_error("[cnt_grow_size] Could not init core");
                vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
                return -1;
            }
            core_count += 1;
            mem_copy(data + core_count * sizeof(uint32_t), &core_sid, sizeof(uint32_t));
            to_grow -= 1;
        }
    }

    vdisk_unload_sector(vdisk, loca_sid, data, SAVE);
    return 0;
}

int fs_cnt_set_size(filesys_t *filesys, uint32_t head_sid, uint32_t size) {
    vdisk_t *vdisk;
    uint8_t *data;

    if (filesys == NULL)
        filesys = MAIN_FS;

    vdisk = fs_get_vdisk(filesys, SID_DISK(head_sid));

    if (vdisk == NULL || !vdisk_is_sector_used(vdisk, head_sid)) {
        sys_warning("[cnt_set_size] Invalid sector id");
        return 1;
    }

    // check if sector is cnt header
    data = vdisk_load_sector(vdisk, head_sid);

    if (data[0] != SF_HEAD) {
        sys_warning("[cnt_set_size] Sector is not container header");
        free(data);
        return 1;
    }

    uint32_t old_count = *((uint32_t *) (data + 1 + META_MAXLEN));
    uint32_t new_count = size / BYTE_IN_CORE;
    old_count = (old_count / BYTE_IN_CORE) + (old_count % BYTE_IN_CORE ? 1 : 0);
    if (size) new_count++;

    uint32_t loca_sid = *((uint32_t *) (data + LAST_SID_OFFSET));
    if (old_count < new_count) {
        // grow cnt
        if (fs_cnt_grow_size(filesys, loca_sid, new_count - old_count)) {
            vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
            return 1;
        }
    } else if (old_count > new_count) {
        // shrink cnt
        int ret = fs_cnt_shrink_size(filesys, loca_sid, old_count - new_count);
        if (ret == -2) ret = 0;
        if (ret) {
            sys_warning("[cnt_set_size] Could not shrink cnt");
            vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
            return 1;
        }
    }

    *((uint32_t *) (data + 1 + META_MAXLEN)) = size;
    vdisk_unload_sector(vdisk, head_sid, data, SAVE);
    return 0;
}

uint32_t fs_cnt_get_size(filesys_t *filesys, uint32_t head_sid) {
    vdisk_t *vdisk;
    uint8_t *data;

    if (filesys == NULL)
        filesys = MAIN_FS;

    vdisk = fs_get_vdisk(filesys, SID_DISK(head_sid));

    if (vdisk == NULL || !vdisk_is_sector_used(vdisk, head_sid)) {
        sys_warning("[cnt_get_size] Invalid sector id");
        return UINT32_MAX;
    }

    // check if sector is cnt header
    data = vdisk_load_sector(vdisk, head_sid);

    if (data[0] != SF_HEAD) {
        sys_warning("[cnt_get_size] Sector is not container header");
        vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
        return UINT32_MAX;
    }

    uint32_t size = *((uint32_t *) (data + 1 + META_MAXLEN));
    vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
    return size;
}
