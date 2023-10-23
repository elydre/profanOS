#include <syscall.h>
#include <i_iolib.h>
#include <type.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    return 0;
}

int access(const char *a, int b) {
    puts("access is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned alarm(unsigned a) {
    puts("alarm is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int chdir(const char *a) {
    puts("chdir is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int chown(const char *a, uid_t b, gid_t c) {
    puts("chown is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int close(int a) {
    puts("close is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
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

int dup(int a) {
    puts("dup is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int dup2(int a, int b) {
    puts("dup2 is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void  encrypt(char a[64], int b) {
    puts("encrypt is not implemented yet, WHY DO YOU USE IT ?");
}

int execl(const char *a, const char *b, ...) {
    puts("execl is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int execle(const char *a, const char *b, ...) {
    puts("execle is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int execlp(const char *a, const char *b, ...) {
    puts("execlp is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int execv(const char *a, char *const *b) {
    puts("execv is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int execve(const char *a, char *const *b, char *const *c) {
    puts("execve is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int execvp(const char *a, char *const *b) {
    puts("execvp is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void  _exit(int a) {
    puts("exit is not implemented yet, WHY DO YOU USE IT ?");
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

pid_t fork() {
    puts("fork is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long  fpathconf(int a, int b) {
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

char *getcwd(char *a, size_t b) {
    puts("getcwd is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

gid_t getegid() {
    puts("getegid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

uid_t geteuid() {
    puts("geteuid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

gid_t getgid() {
    puts("getgid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int getgroups(int a, gid_t *b) {
    puts("getgroups is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long  gethostid() {
    puts("gethostid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int gethostname(char *a, size_t n) {
    puts("gethostname is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *getlogin() {
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

pid_t getpgrp() {
    puts("getpgrp is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t getpid() {
    puts("getpid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t getppid() {
    puts("getppid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t getsid(pid_t a) {
    puts("getsid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

uid_t getuid() {
    puts("getuid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *getwd(char *a) {
    puts("getwd is not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

int isatty(int a) {
    puts("isatty is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
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

off_t lseek(int a, off_t b, int c) {
    puts("lseek is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int nice(int a) {
    puts("nice is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

long  pathconf(const char *a, int b) {
    puts("pathconf is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int pause() {
    puts("pause is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int pipe(int a[2]) {
    puts("pipe is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

ssize_t  pread(int a, void *b, size_t c, off_t d) {
    puts("pread is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

ssize_t  pwrite(int a, const void *b, size_t c, off_t d) {
    puts("pwrite is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

ssize_t  read(int a, void *b, size_t c) {
    puts("read is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

ssize_t  readlink(const char *restrict a, char *restrict b, size_t c) {
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

pid_t setpgrp() {
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

pid_t setsid() {
    puts("setsid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int setuid(uid_t a) {
    puts("setuid is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

unsigned sleep(unsigned a) {
    puts("sleep is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void  swab(const void *restrict a, void *restrict n, ssize_t c) {
    puts("swab is not implemented yet, WHY DO YOU USE IT ?");
}

int symlink(const char *a, const char *b) {
    puts("symlink is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void  sync() {
    puts("sync is not implemented yet, WHY DO YOU USE IT ?");
}

long  sysconf(int a) {
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

useconds_t   ualarm(useconds_t a, useconds_t b) {
    puts("ualarm is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int unlink(const char *a) {
    puts("unlink is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int usleep(useconds_t a) {
    puts("usleep is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

pid_t vfork() {
    puts("vfork is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

ssize_t  write(int a, const void *b, size_t c) {
    puts("write is not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}
