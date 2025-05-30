/*****************************************************************************\
|   === stat.h : 2024 ===                                                     |
|                                                                             |
|    Implementation of the sys/stat.h header file from libC        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <profan/minimal.h>
#include <sys/types.h>

_BEGIN_C_FILE

struct stat {
    dev_t     st_dev;       // ID of device containing file
    ino_t     st_ino;       // file serial number
    mode_t    st_mode;      // mode of file
    nlink_t   st_nlink;     // number of links to the file
    uid_t     st_uid;       // user ID of file
    gid_t     st_gid;       // group ID of file
    dev_t     st_rdev;      // device ID (if file is character or block special)
    off_t     st_size;      // file size in bytes (if file is a regular file)
    time_t    st_atime;     // time of last access
    time_t    st_mtime;     // time of last data modification
    time_t    st_ctime;     // time of last status change
    blksize_t st_blksize;   // optimal blocksize for I/O
    blkcnt_t  st_blocks;    // number of blocks allocated for this object
};

#define S_ISUID 0004000     // set user id on execution
#define S_ISGID 0002000     // set group id on execution
#define S_ISVTX 0001000     // sticky bit

#define S_IRWXU 0000700     // RWX mask for owner
#define S_IRUSR 0000400     // R for owner
#define S_IWUSR 0000200     // W for owner
#define S_IXUSR 0000100     // X for owner

#define S_IRWXG 0000070     // RWX mask for group
#define S_IRGRP 0000040     // R for group
#define S_IWGRP 0000020     // W for group
#define S_IXGRP 0000010     // X for group

#define S_IRWXO 0000007     // RWX mask for other
#define S_IROTH 0000004     // R for other
#define S_IWOTH 0000002     // W for other
#define S_IXOTH 0000001     // X for other

#define S_IFMT   0170000    // type of file mask
#define S_IFBLK  0060000    // block special
#define S_IFCHR  0020000    // character special
#define S_IFIFO  0010000    // FIFO special
#define S_IFREG  0100000    // regular
#define S_IFDIR  0040000    // directory
#define S_IFLNK  0120000    // symbolic link
#define S_IFSOCK 0140000    // socket

#define S_ISBLK(m)  ((m & 0170000) == S_IFBLK)  // block special
#define S_ISCHR(m)  ((m & 0170000) == S_IFCHR)  // character special
#define S_ISFIFO(m) ((m & 0170000) == S_IFIFO)  // FIFO special
#define S_ISREG(m)  ((m & 0170000) == S_IFREG)  // regular
#define S_ISDIR(m)  ((m & 0170000) == S_IFDIR)  // directory
#define S_ISLNK(m)  ((m & 0170000) == S_IFLNK)  // symbolic link
#define S_ISSOCK(m) ((m & 0170000) == S_IFSOCK) // socket

int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);
int fstat(int fd, struct stat *buf);
int lstat(const char *path, struct stat *buf);
int mknod(const char *path, mode_t mode, dev_t dev);
int mkdir(const char *path, mode_t mode);
int mkfifo(const char *path, mode_t mode);
int stat(const char *path, struct stat *buf);
mode_t umask(mode_t mask);

_END_C_FILE

#endif
