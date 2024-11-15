/*****************************************************************************\
|   === unistd.c : 2024 ===                                                   |
|                                                                             |
|    Implementation of unistd functions from libC                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/type.h>
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

int access(const char *a, int b) {
    puts("access is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned alarm(unsigned a) {
    puts("alarm is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int chdir(const char *path) {
    uint32_t sid = fu_path_to_sid(SID_ROOT, path);
    if (!fu_is_dir(sid)) {
        errno = ENOTDIR;
        return -1;
    }

    char *dir = getenv("PWD");
    if (!dir) dir = "/";

    // check if dir exists
    dir = profan_join_path(dir, path);
    fu_simplify_path(dir);

    if (setenv("PWD", dir, 1)) {
        errno = ENOMEM;
        free(dir);
        return -1;
    }

    free(dir);
    return 0;
}

int chown(const char *a, uid_t b, gid_t c) {
    puts("chown is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int close(int fd) {
    return fm_close(fd);
}

size_t confstr(int a, char *b, size_t c) {
    puts("confstr is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *crypt(const char *a, const char *b) {
    puts("crypt is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *ctermid(char *a) {
    puts("ctermid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int dup(int fd) {
    return fm_dup(fd);
}

int dup2(int fd, int newfd) {
    return fm_dup2(fd, newfd);
}

void encrypt(char a[64], int b) {
    puts("encrypt is not implemented yet, WHY DO YOU USE IT ?");
}

int execve(const char *fullpath, char *const argv[], char *const envp[]);
int execl(const char *fullpath, const char *first, ...) {
    va_list args;
    va_start(args, first);

    // count the number of arguments
    int argc = 1;
    const char *arg;
    while ((arg = va_arg(args, const char *)) != NULL) {
        argc++;
    }

    va_end(args);

    // create the argv array
    char **argv = malloc((argc + 1) * sizeof(char *));
    argv[0] = (char *)first;
    va_start(args, first);
    for (int i = 1; i < argc; i++) {
        argv[i] = va_arg(args, char *);
    }
    va_end(args);
    argv[argc] = NULL;

    return execve(fullpath, argv, __get_environ_ptr());
}

int execle(const char *fullpath, const char *arg, ...) {
    puts("execle is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int execlp(const char *file, const char *arg, ...) {
    puts("execlp is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int execv(const char *fullpath, char *const argv[]) {
    return execve(fullpath, argv, NULL);
}

int execve(const char *fullpath, char *const argv[], char *const envp[]) {
    int argc = 0;
    while (argv && argv[argc] != NULL)
        argc++;
    return run_ifexist_full((runtime_args_t) {
        (char *) fullpath,
        argc,
        (char **) argv,
        (char **) envp,
        3
    }, NULL);
}

int execvp(const char *file, char *const argv[]) {
    puts("execvp is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void _exit(int status) {
    exit(status);
}

int fchown(int a, uid_t b, gid_t c) {
    puts("fchown is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int fchdir(int a) {
    puts("fchdir is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int fdatasync(int a) {
    puts("fdatasync is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t fork(void) {
    pid_t pid = syscall_process_fork();

    // error while forking
    if (pid == -1) {
        errno = EAGAIN;
        return -1;
    }

    // child process
    if (pid == 0) {
        return 0;
    }

    // parent process
    syscall_process_wakeup(pid);

    return pid;
}

long fpathconf(int a, int b) {
    puts("fpathconf is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int fsync(int a) {
    puts("fsync is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int ftruncate(int a, off_t b) {
    puts("ftruncate is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *getcwd(char *buf, size_t size) {
    char *working_directory = getenv("PWD");
    size_t wd_len;

    if (!working_directory) {
        errno = ENOMEM;
        return NULL;
    }

    wd_len = strlen(working_directory);

    if (buf == NULL) {
        if (size < wd_len + 1)
            size = wd_len + 1;

        buf = malloc(size);
        strcpy(buf, working_directory);
        return buf;
    }

    if (size < wd_len + 1) {
        errno = ERANGE;
        return NULL;
    }

    strcpy(buf, working_directory);
    return buf;
}

gid_t getegid(void) {
    puts("getegid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

uid_t geteuid(void) {
    // without user system, we just return 0 for 'root'
    return 0;
}

gid_t getgid(void) {
    puts("getgid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int getgroups(int a, gid_t *b) {
    puts("getgroups is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long gethostid(void) {
    puts("gethostid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int gethostname(char *a, size_t n) {
    puts("gethostname is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *getlogin(void) {
    puts("getlogin is not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

int getlogin_r(char *a, size_t n) {
    puts("getlogin_r is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int getopt(int a, char * const *b, const char *c) {
    puts("getopt is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t getpgid(pid_t a) {
    puts("getpgid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t getpgrp(void) {
    puts("getpgrp is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t getpid(void) {
    return syscall_process_pid();
}

pid_t getppid(void) {
    return syscall_process_ppid(syscall_process_pid());
}

pid_t getsid(pid_t a) {
    puts("getsid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

uid_t getuid(void) {
    // withouth user system, we just return 0 for 'root'
    return 0;
}

char *getwd(char *a) {
    puts("getwd is deprecated, use getcwd instead");
    return NULL;
}

int isatty(int fd) {
    return fm_isfctf(fd);
}

int lchown(const char *a, uid_t b, gid_t c) {
    puts("lchown is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int link(const char *a, const char *b) {
    puts("link is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int lockf(int a, int b, off_t c) {
    puts("lockf is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

off_t lseek(int fd, off_t offset, int whence) {
    return fm_lseek(fd, offset, whence);
}

int nice(int a) {
    puts("nice is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long pathconf(const char *a, int b) {
    puts("pathconf is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int pause(void) {
    puts("pause is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int pipe(int fd[2]) {
    return fm_pipe(fd);
}

ssize_t pread(int a, void *b, size_t c, off_t d) {
    puts("pread is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

ssize_t pwrite(int a, const void *b, size_t c, off_t d) {
    puts("pwrite is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

ssize_t read(int fd, void *buf, size_t count) {
    return fm_read(fd, buf, count);
}

ssize_t readlink(const char *restrict a, char *restrict b, size_t c) {
    puts("readlink is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int rmdir(const char *a) {
    puts("rmdir is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int setegid(gid_t a) {
    puts("setegid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int seteuid(uid_t a) {
    puts("seteuid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int setgid(gid_t a) {
    puts("setgid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int setpgid(pid_t a, pid_t b) {
    puts("setpgid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t setpgrp(void) {
    puts("setpgrp is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int setregid(gid_t a, gid_t b) {
    puts("setregid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int setreuid(uid_t a, uid_t b) {
    puts("setreuid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t setsid(void) {
    puts("setsid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int setuid(uid_t a) {
    puts("setuid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned sleep(unsigned seconds) {
    syscall_process_sleep(syscall_process_pid(), seconds * 1000);
    return 0;
}

void swab(const void *restrict a, void *restrict n, ssize_t c) {
    puts("swab is not implemented yet, WHY DO YOU USE IT ?");
}

int symlink(const char *a, const char *b) {
    puts("symlink is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void sync(void) {
    puts("sync is not implemented yet, WHY DO YOU USE IT ?");
}

long sysconf(int a) {
    puts("sysconf is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t tcgetpgrp(int a) {
    puts("tcgetpgrp is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int tcsetpgrp(int a, pid_t b) {
    puts("tcsetpgrp is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int truncate(const char *a, off_t b) {
    puts("truncate is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *ttyname(int a) {
    puts("ttyname is not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

int ttyname_r(int a, char *b, size_t c) {
    puts("ttyname_r is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

useconds_t ualarm(useconds_t a, useconds_t b) {
    puts("ualarm is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int unlink(const char *filename) {
    // add the current working directory to the filename
    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";
    char *path = profan_join_path(pwd, (char *) filename);

    // check if the file exists
    uint32_t parent_sid, elem = fu_path_to_sid(SID_ROOT, path);
    if (!fu_is_file(elem)) {
        fprintf(stderr, "unlink: %s: not a file\n", filename);
        free(path);
        return -1;
    }

    // get the parent directory sid
    char *parent;
    profan_sep_path(path, &parent, NULL);
    parent_sid = fu_path_to_sid(SID_ROOT, parent);
    free(parent);
    free(path);

    if (IS_SID_NULL(parent_sid)) {
        fprintf(stderr, "unlink: %s: parent not found\n", filename);
        return -1;
    }

    // remove the element from the parent directory
    fu_remove_from_dir(parent_sid, elem);

    // delete the file content
    if (syscall_fs_delete(NULL, elem)) {
        fprintf(stderr, "unlink: %s: failed to delete\n", filename);
        return -1;
    }

    return 0;
}

int usleep(useconds_t usec) {
    return syscall_process_sleep(syscall_process_pid(), usec / 1000) ? -1 : 0;
}

pid_t vfork(void) {
    puts("vfork is not implemented yet, WHY DO YOU USE IT ?");
    return -1;
}

ssize_t write(int fd, const void *buf, size_t count) {
    return fm_write(fd, (void *) buf, count);
}
