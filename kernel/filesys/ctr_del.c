/*****************************************************************************\
|   === ctr_del.c : 2024 ===                                                  |
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

int fs_cnt_delete_core(filesys_t *filesys, uint32_t core_sid) {
    vdisk_t *vdisk;
    uint8_t *data;

    vdisk = fs_get_vdisk(filesys, SID_DISK(core_sid));

    if (vdisk == NULL) {
        return 1;
    }

    // check if sector is used
    if (!vdisk_is_sector_used(vdisk, core_sid)) {
        return 1;
    }

    // check if sector is core
    data = vdisk_load_sector(vdisk, core_sid);

    if (data[0] != SF_CORE) {
        vdisk_unload_sector(vdisk, core_sid, data, NO_SAVE);
        return 1;
    }

    // delete core
    vdisk_note_sector_unused(vdisk, core_sid);
    vdisk_unload_sector(vdisk, core_sid, data, SAVE);

    return 0;
}

int fs_cnt_delete_loca_recur(filesys_t *filesys, uint32_t loca_sid) {
    vdisk_t *vdisk;
    uint8_t *data;
    uint32_t next_loca_sid;

    vdisk = fs_get_vdisk(filesys, SID_DISK(loca_sid));

    if (vdisk == NULL) {
        return 1;
    }

    // check if sector is used
    if (!vdisk_is_sector_used(vdisk, loca_sid)) {
        return 1;
    }

    // check if sector is locator
    data = vdisk_load_sector(vdisk, loca_sid);

    if (data[0] != SF_LOCA) {
        vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
        return 1;
    }

    // delete all cores
    for (uint32_t i = 0; i < LINKS_IN_LOCA; i++) {
        uint32_t core_sid = *((uint32_t *) (data + (i + 1) * sizeof(uint32_t)));
        if (IS_SID_NULL(core_sid)) {
            break;
        }
        if (fs_cnt_delete_core(filesys, core_sid)) {
            vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
            return 1;
        }
    }

    // delete next locator
    next_loca_sid = *((uint32_t *) (data + LAST_SID_OFFSET));
    if (!IS_SID_NULL(next_loca_sid)) {
        if (fs_cnt_delete_loca_recur(filesys, next_loca_sid)) {
            vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
            return 1;
        }
    }

    // delete locator
    vdisk_note_sector_unused(vdisk, loca_sid);
    vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
    return 0;
}

int fs_cnt_delete(filesys_t *filesys, uint32_t head_sid) {
    vdisk_t *vdisk;
    uint8_t *data;
    uint32_t loca_sid;

    if (filesys == NULL)
        filesys = MAIN_FS;

    vdisk = fs_get_vdisk(filesys, SID_DISK(head_sid));

    if (vdisk == NULL || !vdisk_is_sector_used(vdisk, head_sid)) {
        sys_warning("[cnt_delete] Invalid sector id");
        return 1;
    }

    // check if sector is cnt header
    data = vdisk_load_sector(vdisk, head_sid);

    if (data[0] != SF_HEAD) {
        sys_warning("[cnt_delete] Sector not container header");
        vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
        return 1;
    }

    // delete locator
    loca_sid = *((uint32_t *) (data + LAST_SID_OFFSET));
    if (!IS_SID_NULL(loca_sid)) {
        if (fs_cnt_delete_loca_recur(filesys, loca_sid)) {
            sys_error("[cnt_delete] Failed to delete locator");
            vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
            return 1;
        }
    }

    // delete cnt header
    vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
    vdisk_note_sector_unused(vdisk, head_sid);
    return 0;
}
