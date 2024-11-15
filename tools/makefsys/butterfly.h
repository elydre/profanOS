/*****************************************************************************\
|   === butterfly.h : 2024 ===                                                |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef BUTTERFLY_H
#define BUTTERFLY_H

#include <stdint.h>

#define SECTOR_SIZE 512
#define META_MAXLEN 64

#define LAST_SID_OFFSET (SECTOR_SIZE - sizeof(uint32_t))
#define LINKS_IN_LOCA ((int) (SECTOR_SIZE / sizeof(uint32_t) - 2))
#define BYTE_IN_CORE (SECTOR_SIZE - 1)

#define SID_FORMAT(disk, sector) ((uint32_t) (((disk) << 24) | (sector)))
#define SID_DISK(sid) ((sid) >> 24)
#define SID_SECTOR(sid) ((sid) & 0xFFFFFF)

#define SID_NULL 0
#define SID_ROOT SID_FORMAT(2, 0)

#define FS_DISKS 256

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 0xffffffff
#endif

// sector functions
#define SF_HEAD 1
#define SF_LOCA 2
#define SF_CORE 3

#define SAVE 1
#define NO_SAVE 0

typedef struct {
    uint8_t data[SECTOR_SIZE];  // sector data
} sector_t;

typedef struct {
    sector_t **sectors;         // array of sectors
    uint32_t size;              // sector count

    uint8_t *used;              // array sectors (bool)
    uint32_t *free;             // array of free sector ids
    uint32_t used_count;        // used sector count
} vdisk_t;

typedef struct {
    vdisk_t **vdisk;            // list mounted virtual disks
    uint32_t vdisk_count;       // virtual disk count
} filesys_t;


#define fs_cnt_read(fs, head_sid, buf, offset, size) fs_cnt_rw(fs, head_sid, buf, offset, size, 1)
#define fs_cnt_write(fs, head_sid, buf, offset, size) fs_cnt_rw(fs, head_sid, buf, offset, size, 0)

#define fs_cnt_init_loca_in_sector(vdisk, sid) fs_cnt_init_sector(vdisk, sid, SF_LOCA)
#define fs_cnt_init_core_in_sector(vdisk, sid) fs_cnt_init_sector(vdisk, sid, SF_CORE)

#define IS_SID_NULL(sid) (sid == SID_NULL)
#define IS_SAME_SID(sid1, sid2) (sid1 == sid2)

// vdisk.c
int      vdisk_note_sector_used(vdisk_t *vdisk, uint32_t sid);
int      vdisk_note_sector_unused(vdisk_t *vdisk, uint32_t sid);
int      vdisk_get_unused_sector(vdisk_t *vdisk);
int      vdisk_extend(vdisk_t *vdisk, uint32_t newsize);
vdisk_t *vdisk_create(uint32_t initsize);
void     vdisk_destroy(vdisk_t *vdisk);
int      vdisk_is_sector_used(vdisk_t *vdisk, uint32_t sid);
int      vdisk_write_sector(vdisk_t *vdisk, uint32_t sid, uint8_t *data);
uint8_t *vdisk_load_sector(vdisk_t *vdisk, uint32_t sid);
int      vdisk_unload_sector(vdisk_t *vdisk, uint32_t sid, uint8_t *data, int save);

// fstools.c
void     sep_path(char *fullpath, char **parent, char **cnt);
void     fs_print_sector(filesys_t *fs, uint32_t sid, int verbose);
vdisk_t *fs_get_vdisk(filesys_t *fs, uint32_t device_id);
void     draw_tree(filesys_t *filesys, uint32_t sid, int depth);

// hostio.c
int      save_vdisk(vdisk_t *vdisk, char *filename);
vdisk_t *load_vdisk(char *filename, uint32_t min_size);
int      host_to_internal(filesys_t *filesys, char *extern_path, char *intern_path);
int      internal_to_host(filesys_t *filesys, char *extern_path, char *intern_path);

// cnt_init.c
int      fs_cnt_init_sector(vdisk_t *vdisk, uint32_t sid, int type);
uint32_t    fs_cnt_init(filesys_t *filesys, uint32_t device_id, char *meta);
char    *fs_cnt_get_meta(filesys_t *filesys, uint32_t sid);

// cnt_size.c
int      fs_cnt_set_size(filesys_t *filesys, uint32_t head_sid, uint32_t size);
uint32_t fs_cnt_get_size(filesys_t *filesys, uint32_t head_sid);

// cnt_rw.c
int      fs_cnt_rw(filesys_t *filesys, uint32_t head_sid, void *buf, uint32_t offset, uint32_t size, int is_read);

// usg_dir.c
int      fu_is_dir(filesys_t *filesys, uint32_t dir_sid);
int      fu_get_dir_content(filesys_t *filesys, uint32_t dir_sid, uint32_t **ids, char ***names);
int      fu_add_element_to_dir(filesys_t *filesys, uint32_t dir_sid, uint32_t element_sid, char *name);
uint32_t    fu_dir_create(filesys_t *filesys, int device_id, char *path);

// usg_file.c
int      fu_is_file(filesys_t *filesys, uint32_t dir_sid);
uint32_t    fu_file_create(filesys_t *filesys, int device_id, char *path);

// usg_ptsid.c
uint32_t    fu_path_to_sid(filesys_t *filesys, uint32_t from, char *path);

#endif
