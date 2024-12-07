/*****************************************************************************\
|   === unistd.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the unistd.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>

#undef  SEEK_SET
#define SEEK_SET 0

#undef  SEEK_CUR
#define SEEK_CUR 1

#undef  SEEK_END
#define SEEK_END 2

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4

int      access(const char *pathname, int mode);
unsigned alarm(unsigned a);
int      chdir(const char *a);
int      chown(const char *a, uid_t b, gid_t c);
int      close(int fd);
size_t   confstr(int a, char *b, size_t c);
char    *crypt(const char *a, const char *b);
char    *ctermid(char *a);
int      dup(int fd);
int      dup2(int fd, int newfd);
void     encrypt(char a[64], int b);
int      execle(const char *fullpath, const char *arg, ...) __attribute__((sentinel));
int      execl(const char *fullpath, const char *first, ...) __attribute__((sentinel));
int      execlp(const char *file, const char *arg, ...) __attribute__((sentinel));
int      execv(const char *fullpath, char *const argv[]);
int      execve(const char *fullpath, char *const argv[], char *const envp[]);
int      execvp(const char *file, char *const argv[]);
void    _exit(int a) __attribute__((noreturn));
int      fchdir(int a);
int      fchown(int a, uid_t b, gid_t c);
int      fdatasync(int a);
pid_t    fork(void);
long     fpathconf(int a, int b);
int      fsync(int a);
int      ftruncate(int a, off_t b);
char    *getcwd(char *a, size_t b);
gid_t    getegid(void);
uid_t    geteuid(void);
gid_t    getgid(void);
int      getgroups(int a, gid_t *b);
long     gethostid(void);
int      gethostname(char *a, size_t n);
char    *getlogin(void);
int      getlogin_r(char *a, size_t n);
int      getopt(int a, char * const *b, const char *c);
pid_t    getpgid(pid_t a);
pid_t    getpgrp(void);
pid_t    getpid(void);
pid_t    getppid(void);
pid_t    getsid(pid_t a);
uid_t    getuid(void);
char    *getwd(char *a);
int      isatty(int fd);
int      lchown(const char *a, uid_t b, gid_t c);
int      link(const char *a, const char *b);
int      lockf(int a, int b, off_t c);
off_t    lseek(int fd, off_t offset, int whence);
int      nice(int a);
long     pathconf(const char *a, int b);
int      pause(void);
int      pipe(int fd[2]);
ssize_t  pread(int a, void *b, size_t c, off_t d);
ssize_t  pwrite(int a, const void *b, size_t c, off_t d);
ssize_t  read(int fd, void *buf, size_t count);
ssize_t  readlink(const char *restrict a, char *restrict b, size_t c);
int      rmdir(const char *a);
int      setegid(gid_t a);
int      seteuid(uid_t a);
int      setgid(gid_t a);
int      setpgid(pid_t a, pid_t b);
pid_t    setpgrp(void);
int      setregid(gid_t a, gid_t b);
int      setreuid(uid_t a, uid_t b);
pid_t    setsid(void);
int      setuid(uid_t a);
unsigned sleep(unsigned seconds);
void     swab(const void *restrict a, void *restrict n, ssize_t c);
int      symlink(const char *a, const char *b);
void     sync(void);
long     sysconf(int a);
pid_t    tcgetpgrp(int a);
int      tcsetpgrp(int a, pid_t b);
int      truncate(const char *a, off_t b);
char    *ttyname(int a);
int      ttyname_r(int a, char *b, size_t c);
useconds_t ualarm(useconds_t a, useconds_t b);
int      unlink(const char *a);
int      usleep(useconds_t usec);
pid_t    vfork(void);
ssize_t  write(int fd, const void *buf, size_t count);

#endif
