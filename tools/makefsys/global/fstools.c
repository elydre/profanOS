#include "../butterfly.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void sep_path(char *fullpath, char **parent, char **cnt) {
    int i, len;

    *parent = (char *)malloc(META_MAXLEN);
    *cnt = (char *)malloc(META_MAXLEN);

    len = strlen(fullpath);
    if (len == 0 || (len == 1 && fullpath[0] == '/')) {
        (*parent)[0] = '\0';
        strcpy((*cnt), "/");
        return;
    }

    for (i = len - 1; i >= 0; i--) {
        if (fullpath[i] == '/') {
            break;
        }
    }

    if (i <= 0) {
        strcpy(*parent, "/");
        strncpy(*cnt, fullpath + 1 + i, META_MAXLEN);
    } else {
        strncpy(*parent, fullpath, i);
        (*parent)[i] = '\0';
        strncpy(*cnt, fullpath + i + 1, META_MAXLEN);
    }
}

void put_printable(char *str, int max) {
    int i;
    for (i = 0; i < max; i++) {
        if (str[i] == '\0') {
            printf("_");
        } else if (str[i] < 32 || str[i] > 126) {
            printf(".");
        } else {
            printf("%c", str[i]);
        }
    }
}

void fs_print_sector(filesys_t *fs, sid_t sid, int verbose) {
    vdisk_t *vdisk;
    sid_t link_sid;

    printf("d%ds%d: ", sid.device, sid.sector);
    if ((sid.device - 1) >= fs->vdisk_count || fs->vdisk[sid.device - 1] == NULL) {
        printf("not mounted\n");
        return;
    }

    vdisk = fs->vdisk[sid.device - 1];
    if (sid.sector >= vdisk->size) {
        printf("out of range\n");
        return;
    }
    if (vdisk_is_sector_used(vdisk, sid)) {
        printf("used, ");
    } else {
        printf("unused\n");
        return;
    }

    uint8_t *data = vdisk_load_sector(vdisk, sid);

    if (data[0] == ST_CONT) {
        printf("type: container");
    } else if (data[0] == ST_SPAR) {
        printf("type: array special");
    } else if (data[0] == ST_MEMS) {
        printf("type: memory pointer");
    } else {
        printf("type: unknown (%d)", data[0]);
    }

    if (data[1] == SF_HEAD) {
        printf(" header, meta: '%s', size %d", data + 2, *((uint32_t *) (data + 2 + META_MAXLEN)));
    } else if (data[1] == SF_LOCA) {
        printf(" locator");
    } else if (data[1] == SF_CORE) {
        printf(" content");
    } else {
        printf(" unknown (%d)", data[1]);
    }

    link_sid.device = data[SECTOR_SIZE - 8] | (data[SECTOR_SIZE - 7] << 8)
        | (data[SECTOR_SIZE - 6] << 16) | (data[SECTOR_SIZE - 5] << 24);

    link_sid.sector = data[SECTOR_SIZE - 4] | (data[SECTOR_SIZE - 3] << 8)
        | (data[SECTOR_SIZE - 2] << 16) | (data[SECTOR_SIZE - 1] << 24);

    if (data[1] != SF_CORE && (link_sid.device || link_sid.sector)) {
        printf(", link: d%ds%d", link_sid.device, link_sid.sector);
    } else if (data[1] != SF_CORE) {
        printf(", link: NULL");
    }

    if (!verbose) {
        printf("\n");
        vdisk_unload_sector(vdisk, sid, data, NO_SAVE);
        return;
    }

    for (uint32_t i = 0; i < SECTOR_SIZE / 16; i++) {
        printf("\n%04x: ", i * 16);
        for (int j = 0; j < 16; j++) {
            printf("%02x ", data[i * 16 + j]);
            if (j % 4 == 3) {
                printf(" ");
            }
        }
        printf(" ");
        put_printable((char *) data + i * 16, 16);
    }

    printf("\n");
    vdisk_unload_sector(vdisk, sid, data, NO_SAVE);
}

vdisk_t *fs_get_vdisk(filesys_t *fs, uint32_t device_id) {
    device_id -= 1;
    if (device_id >= FS_DISKS || fs->vdisk[device_id] == NULL) {
        return NULL;
    }
    return fs->vdisk[device_id];
}

void draw_tree(filesys_t *filesys, sid_t sid, int depth) {
    char **names;
    sid_t *sids;
    int count;

    count = fu_get_dir_content(filesys, sid, &sids, &names);

    if (count == -1) {
        printf("failed to get directory content during path search\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < depth; j++) {
            printf("  ");
        }
        printf("%s, d%ds%d: %dB\n", names[i], sids[i].device, sids[i].sector, fs_cnt_get_size(filesys, sids[i]));
        if (fu_is_dir(filesys, sids[i])) {
            draw_tree(filesys, sids[i], depth + 1);
        }
    }

    for (int i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
    free(sids);
}
