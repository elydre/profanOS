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
#include <stddef.h>

#define SECTOR_SIZE 512
#define META_MAXLEN 64

#undef min
#define min(a, b) ((a) < (b) ? (a) : (b))

#define LINKS_IN_LOCA ((int) ((SECTOR_SIZE / (sizeof(uint32_t) * 2)) - sizeof(uint32_t)))

#define SID_FORMAT(disk, sector) ((uint32_t) (((disk) << 24) | (sector)))
#define SID_DISK(sid) ((sid) >> 24)
#define SID_SECTOR(sid) ((sid) & 0xFFFFFF)

#define SF_HEAD 0xAEFCEBDA // CG - 4836

typedef uint32_t sid_t;

#define SID_NULL 0
#define SID_ROOT SID_FORMAT(1, 1)

#define IS_SID_NULL(sid) ((sid) == SID_NULL)

///////////////////// VDISK

void vdisk_init(void);
void vdisk_destroy(void);
int  vdisk_extend(uint32_t newsize);
int  vdisk_write(void *data, uint32_t size, uint32_t offset);
int  vdisk_read(void *buffer, uint32_t size, uint32_t offset);

//////////////////// HOST I/O

int hio_raw_export(const char *filename);
int hio_raw_import(const char *filename);

int hio_dir_import(const char *extern_path, const char *intern_path);
int hio_dir_export(const char *extern_path, const char *intern_path);

//////////////////// SECTOR

int fs_sector_get_unused(int disk_id);
int fs_sector_note_used(sid_t sid);
int fs_sector_note_free(sid_t sid);

//////////////////// CONTAINER

// cnt_init.c
sid_t fs_cnt_init(uint32_t device_id, const char *meta);
int   fs_cnt_meta(sid_t sid, char *buffer, int buffer_size, int replace);

// cnt_size.c
int      fs_cnt_set_size(uint32_t head_sid, uint32_t size);
uint32_t fs_cnt_get_size(uint32_t head_sid);

// cnt_rw.c
int      fs_cnt_rw(sid_t head_sid, void *buf, uint32_t offset, uint32_t size, int is_read);
#define fs_cnt_read(head_sid, buf, offset, size) fs_cnt_rw(head_sid, buf, offset, size, 1)
#define fs_cnt_write(head_sid, buf, offset, size) fs_cnt_rw(head_sid, buf, offset, size, 0)

/////////////////// USAGE

// usg_tools.c
void     fu_sep_path(const char *fullpath, char **parent, char **cnt);
void     fu_draw_tree(sid_t sid, int depth);

// usg_dir.c
int      fu_is_dir(uint32_t dir_sid);
int      fu_dir_get_content(uint32_t dir_sid, uint32_t **ids, char ***names);
int      fu_add_element_to_dir(uint32_t dir_sid, uint32_t element_sid, char *name);
uint32_t fu_dir_create(int device_id, char *path);

// usg_file.c
int      fu_is_file(uint32_t dir_sid);
uint32_t fu_file_create(int device_id, char *path);

// usg_ptsid.c
uint32_t fu_path_to_sid(uint32_t from, const char *path);

#endif
