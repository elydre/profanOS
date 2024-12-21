/*****************************************************************************\
|   === filesys.h : 2024 ===                                                  |
|                                                                             |
|    Header for filesystem extension (see wiki/lib_filesys)        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef FILESYS_LIB_ID
#define FILESYS_LIB_ID 1002
#define DEVIO_LIB_ID   1003
#define FMOPEN_LIB_ID  1004

#include <stdint.h>

#undef SID_NULL
#define SID_NULL 0

#undef SID_FORMAT
#define SID_FORMAT(disk, sector) ((uint32_t) (((disk) << 24) | (sector)))

#undef SID_DISK
#define SID_DISK(sid) ((sid) >> 24)

#undef SID_SECTOR
#define SID_SECTOR(sid) ((sid) & 0xFFFFFF)

#undef UINT32_MAX
#define UINT32_MAX 0xFFFFFFFF

#undef META_MAXLEN
#define META_MAXLEN 64

#undef SID_ROOT
#define SID_ROOT SID_FORMAT(1, 0)

#undef IS_SID_NULL
#define IS_SID_NULL(sid) (sid == SID_NULL)

#undef IS_SAME_SID
#define IS_SAME_SID(sid1, sid2) (sid1 == sid2)

#undef FS_MAX_DISKS
#define FS_MAX_DISKS 256

enum {
    FCTF_OPEN = 1,
    FCTF_CLOSE,
    FCTF_READ,
    FCTF_WRITE
};

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#ifndef FILESYS_LIB_C

#define fu_is_dir ((int (*)(uint32_t)) get_func_addr(FILESYS_LIB_ID, 2))
#define fu_get_dir_size ((int (*)(uint32_t)) get_func_addr(FILESYS_LIB_ID, 3))
#define fu_get_dir_elm ((int (*)(uint8_t *, uint32_t, uint32_t, uint32_t *)) get_func_addr(FILESYS_LIB_ID, 4))
#define fu_get_dir_content ((int (*)(uint32_t, uint32_t **, char ***)) get_func_addr(FILESYS_LIB_ID, 5))
#define fu_add_to_dir ((int (*)(uint32_t, uint32_t, char *)) get_func_addr(FILESYS_LIB_ID, 6))
#define fu_remove_from_dir ((int (*)(uint32_t, uint32_t)) get_func_addr(FILESYS_LIB_ID, 7))
#define fu_dir_create ((uint32_t (*)(int, const char *)) get_func_addr(FILESYS_LIB_ID, 8))
#define fu_is_file ((int (*)(uint32_t)) get_func_addr(FILESYS_LIB_ID, 9))
#define fu_file_set_size ((int (*)(uint32_t, uint32_t)) get_func_addr(FILESYS_LIB_ID, 10))
#define fu_file_get_size ((uint32_t (*)(uint32_t)) get_func_addr(FILESYS_LIB_ID, 11))
#define fu_file_create ((uint32_t (*)(int, const char *)) get_func_addr(FILESYS_LIB_ID, 12))
#define fu_file_read ((int (*)(uint32_t, void *, uint32_t, uint32_t)) get_func_addr(FILESYS_LIB_ID, 13))
#define fu_file_write ((int (*)(uint32_t, void *, uint32_t, uint32_t)) get_func_addr(FILESYS_LIB_ID, 14))
#define fu_is_fctf ((int (*)(uint32_t)) get_func_addr(FILESYS_LIB_ID, 15))
#define fu_fctf_create ((uint32_t (*)(int, const char *, int (*)(int, void *, uint32_t, uint8_t))) get_func_addr(FILESYS_LIB_ID, 16))
#define fu_fctf_get_addr ((void *(*)(uint32_t)) get_func_addr(FILESYS_LIB_ID, 17))

#define fu_path_to_sid ((uint32_t (*)(uint32_t, const char *)) get_func_addr(FILESYS_LIB_ID, 18))
#define fu_simplify_path ((void (*)(char *)) get_func_addr(FILESYS_LIB_ID, 19))
#define fu_get_vdisk_info ((uint32_t *(*)(void)) get_func_addr(FILESYS_LIB_ID, 20))

#endif

#undef  SEEK_SET
#define SEEK_SET 0

#undef  SEEK_CUR
#define SEEK_CUR 1

#undef  SEEK_END
#define SEEK_END 2

#ifndef FMOPEN_LIB_C

#define fm_open(path, flags) fm_reopen(-1, path, flags)

#define fm_close ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 2))
#define fm_reopen ((int (*)(int, const char *, int)) get_func_addr(FMOPEN_LIB_ID, 3))
#define fm_read ((int (*)(int, void *, uint32_t)) get_func_addr(FMOPEN_LIB_ID, 4))
#define fm_write ((int (*)(int, void *, uint32_t)) get_func_addr(FMOPEN_LIB_ID, 5))
#define fm_lseek ((int (*)(int, int, int)) get_func_addr(FMOPEN_LIB_ID, 6))
#define fm_dup2 ((int (*)(int, int)) get_func_addr(FMOPEN_LIB_ID, 7))
#define fm_dup ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 8))
#define fm_pipe ((int (*)(int[2])) get_func_addr(FMOPEN_LIB_ID, 9))
#define fm_isfctf ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 10))
#define fm_isfile ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 11))
#define fm_newfd_after ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 12))
#define fm_declare_child ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 13))

#endif
#endif
