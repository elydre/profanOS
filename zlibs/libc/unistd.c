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

#define _PROFAN_NO_WD
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

uint32_t profan_wd_sid = SID_ROOT;
char *profan_wd_path;

int   opterr = 1, // if error message should be printed
      optind = 1, // index into parent argv vector
      optopt;     // character checked for validity
char *optarg;     // argument associated with option

void __unistd_init(void) {
    static char wd_path[PROFAN_PATH_MAX] = "/";
    profan_wd_path = wd_path;
}

int access(const char *pathname, int mode) {
    // add the current working directory to the filename
    uint32_t elem = profan_path_resolve(pathname);

    // check if path exists
    if (IS_SID_NULL(elem)) {
        errno = ENOENT;
        return -1;
    }

    // everything is fine (dog meme)
    return 0;
}

unsigned alarm(unsigned a) {
    return (PROFAN_FNI, 0);
}

int chdir(const char *path) {
    uint32_t sid;
    char *dir;

    // check if dir exists
    dir = profan_path_join(profan_wd_path, path);
    fu_simplify_path(dir);

    sid = fu_path_to_sid(SID_ROOT, dir);

    if (!fu_is_dir(sid)) {
        errno = ENOTDIR;
        free(dir);
        return -1;
    }

    // update the working directory
    strlcpy(profan_wd_path, dir, PROFAN_PATH_MAX);
    profan_wd_sid = sid;

    free(dir);
    return 0;
}

int chown(const char *path, uid_t owner, gid_t group) {
    if (IS_SID_NULL(profan_path_resolve(path))) {
        errno = ENOENT;
        return -1;
    }

    return 0;
}

int close(int fd) {
    int r = fm_close(fd);
    if (r < 0) {
        errno = -r;
        return -1;
    }
    return 0;
}

size_t confstr(int a, char *b, size_t c) {
    return (PROFAN_FNI, 0);
}

char *crypt(const char *a, const char *b) {
    return (PROFAN_FNI, NULL);
}

char *ctermid(char *a) {
    char *term = getenv("TERM");
    if (term == NULL)
        term = "/dev/panda";
    if (a == NULL)
        return term;
    strlcpy(a, term, L_ctermid);
    return a;
}

int dup(int fd) {
    int newfd = fm_dup(fd);
    if (newfd < 0) {
        errno = -newfd;
        return -1;
    }
    return newfd;
}

int dup2(int fd, int newfd) {
    int r = fm_dup2(fd, newfd);
    if (r < 0) {
        errno = -r;
        return -1;
    }
    return newfd;
}

void encrypt(char a[64], int b) {
    PROFAN_FNI;
}

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

    return execve(fullpath, argv, environ);
}

int execle(const char *fullpath, const char *arg, ...) {
    return (PROFAN_FNI, 0);
}

int execlp(const char *file, const char *arg, ...) {
    return (PROFAN_FNI, 0);
}

int execv(const char *fullpath, char *const argv[]) {
    return execve(fullpath, argv, environ);
}

int execve(const char *fullpath, char *const argv[], char *const envp[]) {
    int argc = 0;
    while (argv && argv[argc] != NULL)
        argc++;
    run_ifexist_full((runtime_args_t) {
        (char *) fullpath,
        profan_wd_path,
        argc,
        (char **) argv,
        (char **) envp,
        3
    }, NULL);

    errno = ENOENT;
    return -1;
}

int execvp(const char *file, char *const argv[]) {
    char *fullpath = profan_path_path(file, 1);
    if (fullpath)
        return execve(fullpath, argv, environ);
    errno = ENOENT;
    return -1;
}

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    char *fullpath = profan_path_path(file, 1);
    if (fullpath)
        return execve(fullpath, argv, envp);
    errno = ENOENT;
    return -1;
}

void _exit(int status) {
    syscall_process_kill(syscall_process_pid(), status);
    while (1); // unreachable (probably)
}

int fchown(int fd, uid_t owner, gid_t group) {
    return 0;
}

int fchdir(int a) {
    const char *path = fm_get_path(a);
    if (path == NULL) {
        errno = EBADF;
        return -1;
    }
    return chdir(path);
}

int fdatasync(int a) {
    return (PROFAN_FNI, 0);
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

    fm_declare_child(pid);

    // parent process
    syscall_process_wakeup(pid, 0);

    return pid;
}

long fpathconf(int a, int b) {
    return (PROFAN_FNI, 0);
}

int fsync(int a) {
    return (PROFAN_FNI, 0);
}

int ftruncate(int fd, off_t length) {
    uint32_t sid = fm_get_sid(fd);

    if (!fu_is_file(sid)) {
        errno = EINVAL;
        return -1;
    }

    if (syscall_fs_set_size(NULL, sid, length)) {
        errno = EIO;
        return -1;
    }

    return 0;
}

char *getcwd(char *buf, size_t size) {
    size_t wd_len;

    wd_len = strlen(profan_wd_path);

    if (buf == NULL) {
        if (size < wd_len + 1)
            size = wd_len + 1;

        buf = malloc(size);
        strcpy(buf, profan_wd_path);
        return buf;
    }

    if (size < wd_len + 1) {
        errno = ERANGE;
        return NULL;
    }

    strcpy(buf, profan_wd_path);
    return buf;
}

// without user system, we just return 0 for "root"

gid_t getegid(void) {
    return 0;
}

uid_t geteuid(void) {
    return 0;
}

gid_t getgid(void) {
    return 0;
}

int getgroups(int a, gid_t *b) {
    return (PROFAN_FNI, 0);
}

long gethostid(void) {
    return (PROFAN_FNI, 0);
}

int gethostname(char *a, size_t n) {
    return (PROFAN_FNI, 0);
}

char *getlogin(void) {
    return (PROFAN_FNI, NULL);
}

int getlogin_r(char *a, size_t n) {
    return (PROFAN_FNI, 0);
}

int getopt(int nargc, char *const *nargv, const char *ostr) {
    static char *place = "";
    char *oli;

    if (!*place) {
        if (optind >= nargc || *(place = nargv[optind]) != '-') {
            place = "";
            return -1;
        }

        if (place[1] && *++place == '-' && place[1] == '\0') {
            ++optind;
            place = "";
            return -1;
        }
    }

    if ((optopt = (int) *place++) == (int) ':' || !(oli = strchr(ostr, optopt))) {
        if (optopt == (int) '-') {
            place = "";
            return -1;
        }
        if (!*place)
            optind++;
        if (opterr && *ostr != ':')
            fprintf(stderr, "illegal option -- '%c'\n", optopt);
        return (int) '?';
    }

    if (*++oli != ':') {
        optarg = NULL;
        if (!*place)
            optind++;
    } else {
        if (*place) {
            optarg = place;
        } else if (nargc <= ++optind) {
            place = "";
            if (*ostr == ':')
                return (int) ':';
            if (opterr)
                fprintf(stderr, "option requires an argument -- '%c'\n", optopt);
            return (int) '?';
        } else {
            optarg = nargv[optind];
        }
        place = "";
        optind++;
    }

    return optopt;
}

pid_t getpgid(pid_t a) {
    return (PROFAN_FNI, 0);
}

pid_t getpgrp(void) {
    return (PROFAN_FNI, 0);
}

pid_t getpid(void) {
    return syscall_process_pid();
}

pid_t getppid(void) {
    return syscall_process_ppid(syscall_process_pid());
}

pid_t getsid(pid_t a) {
    return (PROFAN_FNI, 0);
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
    return fm_isfctf(fd) > 0;
}

int lchown(const char *path, uid_t owner, gid_t group) {
    if (IS_SID_NULL(profan_path_resolve(path))) {
        errno = ENOENT;
        return -1;
    }

    return 0;
}

int link(const char *a, const char *b) {
    return (PROFAN_FNI, 0);
}

int lockf(int a, int b, off_t c) {
    return (PROFAN_FNI, 0);
}

off_t lseek(int fd, off_t offset, int whence) {
    int r = fm_lseek(fd, offset, whence);
    if (r < 0) {
        errno = -r;
        return -1;
    }
    return r;
}

int nice(int a) {
    return (PROFAN_FNI, 0);
}

long pathconf(const char *a, int b) {
    return (PROFAN_FNI, 0);
}

int pause(void) {
    return (PROFAN_FNI, 0);
}

int pipe(int fd[2]) {
    int r = fm_pipe(fd);
    if (r < 0) {
        errno = -r;
        return -1;
    }
    return 0;
}

ssize_t pread(int a, void *b, size_t c, off_t d) {
    return (PROFAN_FNI, 0);
}

ssize_t pwrite(int a, const void *b, size_t c, off_t d) {
    return (PROFAN_FNI, 0);
}

ssize_t read(int fd, void *buf, size_t count) {
    int r = fm_read(fd, buf, count);
    if (r < 0) {
        errno = -r;
        return -1;
    }
    return r;
}

ssize_t readlink(const char *restrict a, char *restrict b, size_t c) {
    if (profan_path_resolve(a) == SID_NULL)
        errno = ENOENT;
    else
        errno = EINVAL; // no links for now
    return -1;
}

int rmdir(const char *a) {
    return (PROFAN_FNI, 0);
}

int setegid(gid_t a) {
    return (PROFAN_FNI, 0);
}

int seteuid(uid_t a) {
    return (PROFAN_FNI, 0);
}

int setgid(gid_t a) {
    return (PROFAN_FNI, 0);
}

int setpgid(pid_t a, pid_t b) {
    return (PROFAN_FNI, 0);
}

pid_t setpgrp(void) {
    return (PROFAN_FNI, 0);
}

int setregid(gid_t a, gid_t b) {
    return (PROFAN_FNI, 0);
}

int setreuid(uid_t a, uid_t b) {
    return (PROFAN_FNI, 0);
}

pid_t setsid(void) {
    return (PROFAN_FNI, 0);
}

int setuid(uid_t a) {
    return (PROFAN_FNI, 0);
}

unsigned sleep(unsigned seconds) {
    syscall_process_sleep(syscall_process_pid(), seconds * 1000);
    return 0;
}

void swab(const void *restrict a, void *restrict n, ssize_t c) {
    PROFAN_FNI;
}

int symlink(const char *a, const char *b) {
    return (PROFAN_FNI, 0);
}

void sync(void) {
    PROFAN_FNI;
}

long sysconf(int a) {
    return (PROFAN_FNI, 0);
}

pid_t tcgetpgrp(int a) {
    return (PROFAN_FNI, 0);
}

int tcsetpgrp(int a, pid_t b) {
    return (PROFAN_FNI, 0);
}

int truncate(const char *filename, off_t length) {
    uint32_t sid = profan_path_resolve(filename);

    if (!fu_is_file(sid)) {
        errno = EINVAL;
        return -1;
    }

    if (syscall_fs_set_size(NULL, sid, length)) {
        errno = EIO;
        return -1;
    }

    return 0;
}

char *ttyname(int a) {
    return (PROFAN_FNI, NULL);
}

int ttyname_r(int a, char *b, size_t c) {
    return (PROFAN_FNI, 0);
}

useconds_t ualarm(useconds_t a, useconds_t b) {
    return (PROFAN_FNI, 0);
}

int unlink(const char *filename) {
    // add the current working directory to the filename
    char *path = profan_path_join(profan_wd_path, (char *) filename);

    // check if the file exists
    uint32_t parent_sid, elem = fu_path_to_sid(SID_ROOT, path);
    if (!fu_is_file(elem)) {
        errno = ENOENT;
        free(path);
        return -1;
    }

    // get the parent directory sid
    char *parent;
    profan_path_sep(path, &parent, NULL);
    parent_sid = fu_path_to_sid(SID_ROOT, parent);
    free(parent);
    free(path);

    if (IS_SID_NULL(parent_sid)) {
        errno = ENOENT;
        return -1;
    }

    // remove the element from the parent directory
    if (fu_remove_from_dir(parent_sid, elem)) {
        errno = EIO;
        return -1;
    }

    // delete the file content
    if (syscall_fs_delete(NULL, elem)) {
        errno = EIO;
        return -1;
    }

    return 0;
}

int usleep(useconds_t usec) {
    return syscall_process_sleep(syscall_process_pid(), usec / 1000) ? -1 : 0;
}

pid_t vfork(void) {
    return (PROFAN_FNI, -1);
}

ssize_t write(int fd, const void *buf, size_t count) {
    int r = fm_write(fd, (void *) buf, count);
    if (r < 0) {
        errno = -r;
        return -1;
    }
    return r;
}
