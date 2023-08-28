#include <kernel/butterfly.h>
#include <minilib.h>
#include <ktype.h>


void sep_path(char *fullpath, char **parent, char **cnt) {
    int i, len;

    *parent = (char *) malloc(META_MAXLEN);
    *cnt = (char *) malloc(META_MAXLEN);
    (*parent)[0] = '\0';
    (*cnt)[0] = '\0';

    len = str_len(fullpath);

    if (len == 0 || (len == 1 && fullpath[0] == '/')) {
        str_cpy((*cnt), "/");
        return;
    }

    if (fullpath[len - 1] == '/') {
        fullpath[len - 1] = '\0';
        len--;
    }

    for (i = len - 1; i >= 0; i--) {
        if (fullpath[i] == '/') {
            break;
        }
    }

    if (i <= 0) {
        str_cpy(*parent, "/");
        str_ncpy(*cnt, fullpath + 1 + i, META_MAXLEN);
    } else {
        str_ncpy(*parent, fullpath, i);
        (*parent)[i] = '\0';
        str_ncpy(*cnt, fullpath + i + 1, META_MAXLEN);
    }
}


vdisk_t *fs_get_vdisk(filesys_t *fs, uint32_t device_id) {
    device_id -= 1;
    if (device_id >= FS_DISKS || fs->vdisk[device_id] == NULL) {
        return NULL;
    }
    return fs->vdisk[device_id];
}
