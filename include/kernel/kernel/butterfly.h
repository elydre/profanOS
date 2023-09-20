#ifndef BUTTERFLY_H
#define BUTTERFLY_H

#include <ktype.h>

#ifndef FS_SECTOR_SIZE
#define FS_SECTOR_SIZE 256
#endif

#define META_MAXLEN 64

#define LAST_SID_OFFSET (FS_SECTOR_SIZE - sizeof(sid_t))
#define LINKS_IN_LOCA ((int) (FS_SECTOR_SIZE / sizeof(sid_t) - 2))
#define BYTE_IN_CORE (FS_SECTOR_SIZE - 2)

#define NULL_SID ((sid_t){0, 0})
#define ROOT_SID ((sid_t){1, 0})

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 0xffffffff
#endif

// sector types
#define ST_CONT 1
#define ST_SPAR 2
#define ST_MEMS 3

// sector functions
#define SF_HEAD 1
#define SF_LOCA 2
#define SF_CORE 3

#define SAVE 1
#define NO_SAVE 0

#define fs_cnt_read(fs, head_sid, buf, offset, size) fs_cnt_rw(fs, head_sid, buf, offset, size, 1)
#define fs_cnt_write(fs, head_sid, buf, offset, size) fs_cnt_rw(fs, head_sid, buf, offset, size, 0)

#define fs_cnt_init_loca_in_sector(vdisk, sid) fs_cnt_init_sector(vdisk, sid, SF_LOCA)
#define fs_cnt_init_core_in_sector(vdisk, sid) fs_cnt_init_sector(vdisk, sid, SF_CORE)

#define IS_NULL_SID(sid) ((sid).device == 0 && (sid).sector == 0)
#define IS_SAME_SID(sid1, sid2) ((sid1).device == (sid2).device && (sid1).sector == (sid2).sector)

// vdisk.c
int      vdisk_note_sector_used(vdisk_t *vdisk, sid_t sid);
int      vdisk_note_sector_unused(vdisk_t *vdisk, sid_t sid);
int      vdisk_get_unused_sector(vdisk_t *vdisk);
int      vdisk_extend(vdisk_t *vdisk, uint32_t newsize);
vdisk_t *vdisk_create(uint32_t initsize);
void     vdisk_destroy(vdisk_t *vdisk);
int      vdisk_is_sector_used(vdisk_t *vdisk, sid_t sid);
int      vdisk_write_sector(vdisk_t *vdisk, sid_t sid, uint8_t *data);
int      vdisk_read_sector(vdisk_t *vdisk, sid_t sid, uint8_t *data);
uint8_t *vdisk_load_sector(vdisk_t *vdisk, sid_t sid);
int      vdisk_unload_sector(vdisk_t *vdisk, sid_t sid, uint8_t *data, int save);

// fstools.c
void     sep_path(char *fullpath, char **parent, char **cnt);
vdisk_t *fs_get_vdisk(filesys_t *fs, uint32_t device_id);

// cnt_init.c
int      fs_cnt_init_sector(vdisk_t *vdisk, sid_t sid, int type);
sid_t    fs_cnt_init(filesys_t *filesys, uint32_t device_id, char *meta);
char    *fs_cnt_get_meta(filesys_t *filesys, sid_t sid);
void     fs_cnt_change_meta(filesys_t *filesys, sid_t sid, char *meta);

// cnt_size.c
int      fs_cnt_set_size(filesys_t *filesys, sid_t head_sid, uint32_t size);
uint32_t fs_cnt_get_size(filesys_t *filesys, sid_t head_sid);

// cnt_del.c
int      fs_cnt_delete(filesys_t *filesys, sid_t head_sid);

// cnt_rw.c
int      fs_cnt_rw(filesys_t *filesys, sid_t head_sid, void *buf, uint32_t offset, uint32_t size, int is_read);

// usg_dir.c
int      fu_is_dir(filesys_t *filesys, sid_t dir_sid);
int      fu_get_dir_content(filesys_t *filesys, sid_t dir_sid, sid_t **ids, char ***names);
int      fu_add_element_to_dir(filesys_t *filesys, sid_t dir_sid, sid_t element_sid, char *name);
sid_t    fu_dir_create(filesys_t *filesys, int device_id, char *path);

// usg_file.c
int      fu_is_file(filesys_t *filesys, sid_t dir_sid);
sid_t    fu_file_create(filesys_t *filesys, int device_id, char *path);

// usg_ptsid.c
sid_t    fu_path_to_sid(filesys_t *filesys, sid_t from, char *path);

// filesys.c
filesys_t *fs_get_main();
int      filesys_init();


#endif
