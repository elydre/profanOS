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
#define FILESYS_LIB_ID 1
#define FMOPEN_LIB_ID  3

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

#ifndef _KERNEL_MODULE

extern int profan_syscall(uint32_t id, ...);

#undef  _pscall
#define _pscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

int       fu_is_dir(uint32_t sid);
int       fu_dir_get_size(uint32_t sid);
int       fu_dir_get_elm(uint8_t *name, uint32_t sid, uint32_t index, uint32_t *elm_sid);
int       fu_dir_get_content(uint32_t sid, uint32_t **elm_sids, char ***elm_names);
int       fu_add_to_dir(uint32_t dir_sid, uint32_t elm_sid, char *name);
int       fu_remove_from_dir(uint32_t dir_sid, uint32_t elm_sid);
uint32_t  fu_dir_create(int disk, const char *path);
int       fu_is_file(uint32_t sid);
int       fu_file_set_size(uint32_t sid, uint32_t size);
uint32_t  fu_file_get_size(uint32_t sid);
uint32_t  fu_file_create(int disk, const char *path);
int       fu_file_read(uint32_t sid, void *buf, uint32_t offset, uint32_t size);
int       fu_file_write(uint32_t sid, void *buf, uint32_t offset, uint32_t size);
int       fu_is_afft(uint32_t sid);
uint32_t  fu_afft_create(int disk, const char *path, int (*handler)(int, void *, uint32_t, uint8_t));
void     *fu_afft_get_addr(uint32_t sid);
uint32_t  fu_path_to_sid(uint32_t disk, const char *path);
uint32_t *fu_get_vdisk_info(void);

#define fu_is_dir(a)                  ((int) _pscall(FILESYS_LIB_ID, 0, a))
#define fu_dir_get_size(a)            ((int) _pscall(FILESYS_LIB_ID, 1, a))
#define fu_dir_get_elm(a, b, c, d)    ((int) _pscall(FILESYS_LIB_ID, 2, a, b, c, d))
#define fu_dir_get_content(a, b, c)   ((int) _pscall(FILESYS_LIB_ID, 3, a, b, c))
#define fu_add_to_dir(a, b, c)        ((int) _pscall(FILESYS_LIB_ID, 4, a, b, c))
#define fu_remove_from_dir(a, b)      ((int) _pscall(FILESYS_LIB_ID, 5, a, b))
#define fu_dir_create(a, b)           ((uint32_t) _pscall(FILESYS_LIB_ID, 6, a, b))
#define fu_is_file(a)                 ((int) _pscall(FILESYS_LIB_ID, 7, a))
#define fu_file_set_size(a, b)        ((int) _pscall(FILESYS_LIB_ID, 8, a, b))
#define fu_file_get_size(a)           ((uint32_t) _pscall(FILESYS_LIB_ID, 9, a))
#define fu_file_create(a, b)          ((uint32_t) _pscall(FILESYS_LIB_ID, 10, a, b))
#define fu_file_read(a, b, c, d)      ((int) _pscall(FILESYS_LIB_ID, 11, a, b, c, d))
#define fu_file_write(a, b, c, d)     ((int) _pscall(FILESYS_LIB_ID, 12, a, b, c, d))
#define fu_is_afft(a)                 ((int) _pscall(FILESYS_LIB_ID, 13, a))
#define fu_afft_create(a, b, c)       ((uint32_t) _pscall(FILESYS_LIB_ID, 14, a, b, c))
#define fu_afft_get_addr(a)           ((void *) _pscall(FILESYS_LIB_ID, 15, a))
#define fu_path_to_sid(a, b)          ((uint32_t) _pscall(FILESYS_LIB_ID, 16, a, b))
#define fu_get_vdisk_info()           ((uint32_t *) _pscall(FILESYS_LIB_ID, 17, 0))

#endif // _KERNEL_MODULE

#define fm_open(path, flags) fm_reopen(-1, path, flags)
#define fm_read(fd, buf, size) fm_pread(fd, buf, size, -1)
#define fm_write(fd, buf, size) fm_pwrite(fd, buf, size, -1)

int fm_close(int fd);
int fm_reopen(int fd, const char *path, int flags);
int fm_pread(int fd, void *buf, uint32_t size, int offset);
int fm_pwrite(int fd, void *buf, uint32_t size, int offset);
int fm_lseek(int fd, int offset, int whence);
int fm_dup2(int oldfd, int newfd);
int fm_dup(int fd);
int fm_pipe(int fds[2]);
int fm_isafft(int fd);
int fm_isfile(int fd);
int fm_fcntl(int fd, int cmd, int arg);
uint32_t fm_get_sid(int fd);
const char *fm_get_path(int fd);
int fm_declare_child(int fd);

#ifndef _KERNEL_MODULE

#define fm_close(a)          ((int) _pscall(FMOPEN_LIB_ID, 0, a))
#define fm_reopen(a, b, c)   ((int) _pscall(FMOPEN_LIB_ID, 1, a, b, c))
#define fm_pread(a, b, c, d)  ((int) _pscall(FMOPEN_LIB_ID, 2, a, b, c, d))
#define fm_pwrite(a, b, c, d) ((int) _pscall(FMOPEN_LIB_ID, 3, a, b, c, d))
#define fm_lseek(a, b, c)    ((int) _pscall(FMOPEN_LIB_ID, 4, a, b, c))
#define fm_dup2(a, b)        ((int) _pscall(FMOPEN_LIB_ID, 5, a, b))
#define fm_dup(a)            ((int) _pscall(FMOPEN_LIB_ID, 6, a))
#define fm_pipe(a)           ((int) _pscall(FMOPEN_LIB_ID, 7, a))
#define fm_isafft(a)         ((int) _pscall(FMOPEN_LIB_ID, 8, a))
#define fm_isfile(a)         ((int) _pscall(FMOPEN_LIB_ID, 9, a))
#define fm_fcntl(a, b, c)    ((int) _pscall(FMOPEN_LIB_ID, 10, a, b, c))
#define fm_get_sid(a)        ((uint32_t) _pscall(FMOPEN_LIB_ID, 11, a))
#define fm_get_path(a)       ((const char *) _pscall(FMOPEN_LIB_ID, 12, a))
#define fm_declare_child(a)  ((int) _pscall(FMOPEN_LIB_ID, 13, a))

#else

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define fm_close ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 0))
#define fm_reopen ((int (*)(int, const char *, int)) get_func_addr(FMOPEN_LIB_ID, 1))
#define fm_pread ((int (*)(int, void *, uint32_t, int)) get_func_addr(FMOPEN_LIB_ID, 2))
#define fm_pwrite ((int (*)(int, void *, uint32_t, int)) get_func_addr(FMOPEN_LIB_ID, 3))
#define fm_lseek ((int (*)(int, int, int)) get_func_addr(FMOPEN_LIB_ID, 4))
#define fm_dup2 ((int (*)(int, int)) get_func_addr(FMOPEN_LIB_ID, 5))
#define fm_dup ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 6))
#define fm_pipe ((int (*)(int[2])) get_func_addr(FMOPEN_LIB_ID, 7))
#define fm_isfctf ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 8))
#define fm_isfile ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 9))
#define fm_fcntl ((int (*)(int, int, int)) get_func_addr(FMOPEN_LIB_ID, 10))
#define fm_get_sid ((uint32_t (*)(int)) get_func_addr(FMOPEN_LIB_ID, 11))
#define fm_get_path ((const char *(*)(int)) get_func_addr(FMOPEN_LIB_ID, 12))
#define fm_declare_child ((int (*)(int)) get_func_addr(FMOPEN_LIB_ID, 13))

#endif // _KERNEL_MODULE

#endif
