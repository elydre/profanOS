/*****************************************************************************\
|   === ctr_init.c : 2024 ===                                                 |
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

int fs_cnt_init_sector(vdisk_t *vdisk, uint32_t sid, int type) {
    uint8_t *data;

    // check if sector unused
    if (vdisk_is_sector_used(vdisk, sid)) {
        printf("d%ds%d already used\n", SID_DISK(sid), SID_SECTOR(sid));
        return 1;
    }

    vdisk_note_sector_used(vdisk, sid);

    data = calloc(SECTOR_SIZE, sizeof(uint8_t));

    // add sector identifier
    data[0] = type;

    vdisk_write_sector(vdisk, sid, data);

    free(data);

    return 0;
}

#define fs_cnt_init_loca_in_sector(vdisk, sid) fs_cnt_init_sector(vdisk, sid, SF_LOCA)
#define fs_cnt_init_core_in_sector(vdisk, sid) fs_cnt_init_sector(vdisk, sid, SF_CORE)

uint32_t fs_cnt_init(filesys_t *filesys, uint32_t device_id, char *meta) {
    vdisk_t *vdisk;
    uint32_t main_sid;
    uint32_t loca_sid;

    uint8_t *data;
    int ret_sect;


    vdisk = fs_get_vdisk(filesys, device_id);
    if (vdisk == NULL) {
        printf("d%d not found\n", device_id);
        return SID_NULL;
    }

    // get unused sector for header
    ret_sect = vdisk_get_unused_sector(vdisk);
    if (ret_sect == -1) {
        printf("no more sectors in d%d\n", device_id);
        return SID_NULL;
    }
    main_sid = SID_FORMAT(device_id, (uint32_t) ret_sect);
    vdisk_note_sector_used(vdisk, main_sid);

    // get unused sector for locator
    ret_sect = vdisk_get_unused_sector(vdisk);
    if (ret_sect == -1) {
        printf("no more sectors in d%d\n", SID_DISK(main_sid));
        vdisk_note_sector_unused(vdisk, main_sid);
        return SID_NULL;
    }
    loca_sid = SID_FORMAT(SID_DISK(main_sid), (uint32_t) ret_sect);

    // init locator
    if (fs_cnt_init_loca_in_sector(vdisk, loca_sid)) {
        printf("failed to init core\n");
        vdisk_note_sector_unused(vdisk, main_sid);
        vdisk_note_sector_unused(vdisk, loca_sid);
        return SID_NULL;
    }

    data = calloc(SECTOR_SIZE, sizeof(uint8_t));

    // add sector identifier
    data[0] = SF_HEAD;

    // add meta and core sid
    memcpy(data + 1, meta, min(strlen(meta), META_MAXLEN - 1));

    memcpy(data + LAST_SID_OFFSET, &loca_sid, sizeof(uint32_t));

    vdisk_write_sector(vdisk, main_sid, data);

    free(data);

    return main_sid;
}

char *fs_cnt_get_meta(filesys_t *filesys, uint32_t sid) {
    vdisk_t *vdisk;
    uint8_t *data;
    char *meta;

    vdisk = fs_get_vdisk(filesys, SID_DISK(sid));
    if (vdisk == NULL) {
        printf("d%d not found\n", SID_DISK(sid));
        return NULL;
    }

    data = vdisk_load_sector(vdisk, sid);
    if (data == NULL) {
        printf("failed to read d%ds%d\n", SID_DISK(sid), SID_SECTOR(sid));
        return NULL;
    }

    meta = calloc(META_MAXLEN, sizeof(char));
    memcpy(meta, data + 1, META_MAXLEN - 1);

    vdisk_unload_sector(vdisk, sid, data, NO_SAVE);

    return meta;
}
