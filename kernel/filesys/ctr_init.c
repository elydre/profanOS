/*****************************************************************************\
|   === ctr_init.c : 2024 ===                                                 |
|                                                                             |
|    Kernel filesystem control functions                           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

#define fs_cnt_init_loca_in_sector(vdisk, sid) fs_cnt_init_sector(vdisk, sid, SF_LOCA)
#define fs_cnt_init_core_in_sector(vdisk, sid) fs_cnt_init_sector(vdisk, sid, SF_CORE)

int fs_cnt_init_sector(vdisk_t *vdisk, uint32_t sid, int type) {
    uint8_t *data;

    // check if sector unused
    if (vdisk_is_sector_used(vdisk, sid)) {
        return 1;
    }

    vdisk_note_sector_used(vdisk, sid);

    data = calloc(FS_SECTOR_SIZE);

    // add sector identifier
    data[0] = type;

    vdisk_write_sector(vdisk, sid, data);

    free(data);

    return 0;
}

uint32_t fs_cnt_init(filesys_t *filesys, uint8_t device_id, char *meta) {
    uint32_t main_sid;
    uint32_t loca_sid;

    vdisk_t *vdisk;
    uint8_t *data;
    int ret_sect;

    if (filesys == NULL)
        filesys = MAIN_FS;

    vdisk = fs_get_vdisk(filesys, device_id);
    if (vdisk == NULL) {
        sys_warning("[cnt_init] vdisk not found");
        return SID_NULL;
    }

    // get unused sector for header
    ret_sect = vdisk_get_unused_sector(vdisk);
    if (ret_sect == -1) {
        sys_error("[cnt_init] No more free sectors");
        return SID_NULL;
    }
    main_sid = SID_FORMAT(device_id, ret_sect);
    vdisk_note_sector_used(vdisk, main_sid);

    // get unused sector for locator
    ret_sect = vdisk_get_unused_sector(vdisk);
    if (ret_sect == -1) {
        sys_error("[cnt_init] No more free sectors");
        vdisk_note_sector_unused(vdisk, main_sid);
        return SID_NULL;
    }
    loca_sid = SID_FORMAT(SID_DISK(main_sid), (uint32_t) ret_sect);

    // init locator
    if (fs_cnt_init_loca_in_sector(vdisk, loca_sid)) {
        sys_error("[cnt_init] Could not init locator");
        vdisk_note_sector_unused(vdisk, main_sid);
        vdisk_note_sector_unused(vdisk, loca_sid);
        return SID_NULL;
    }

    data = calloc(FS_SECTOR_SIZE);

    // add sector identifier
    data[0] = SF_HEAD;

    // add meta and core sid
    mem_copy(data + 1, meta, min(str_len(meta), META_MAXLEN - 1));

    mem_copy(data + LAST_SID_OFFSET, &loca_sid, sizeof(uint32_t));

    vdisk_write_sector(vdisk, main_sid, data);

    free(data);

    return main_sid;
}

char *fs_cnt_meta(filesys_t *filesys, uint32_t sid, char *meta) {
    vdisk_t *vdisk;
    uint8_t *data;

    if (filesys == NULL)
        filesys = MAIN_FS;

    vdisk = fs_get_vdisk(filesys, SID_DISK(sid));

    if (vdisk == NULL || SID_SECTOR(sid) >= vdisk->size)
        return NULL;

    data = vdisk_load_sector(vdisk, sid);

    if (meta) {
        mem_copy(data + 1, meta, META_MAXLEN - 1);
    } else {
        meta = calloc(META_MAXLEN);
        mem_copy(meta, data + 1, META_MAXLEN - 1);
    }

    vdisk_unload_sector(vdisk, sid, data, SAVE);

    return meta;
}
