#include <syscall.h>
#include <i_iolib.h>
#include <i_string.h>
#include <type.h>
#include <stdlib.h>
#include <stdio.h>

void main() {
    printf("Init of the unistd lib !\n");
}

int access(const char *a, int b) {
    printf("access is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

unsigned alarm(unsigned a) {
    printf("alarm is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int chdir(const char *a) {
    printf("chdir is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int chown(const char *a, uid_t b, gid_t c) {
    printf("chown is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int close(int a) {
    printf("close is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

size_t confstr(int a, char *b, size_t c) {
    printf("confstr is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *crypt(const char *a, const char *b) {
    printf("crypt is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *ctermid(char *a) {
    printf("ctermid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int dup(int a) {
    printf("dup is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int dup2(int a, int b) {
    printf("dup2 is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

void  encrypt(char a[64], int b) {
    printf("encrypt is not implemented yet, WHY DO YOU USE IT ?\n");
}

int execl(const char *a, const char *b, ...) {
    printf("execl is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int execle(const char *a, const char *b, ...) {
    printf("execle is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int execlp(const char *a, const char *b, ...) {
    printf("execlp is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int execv(const char *a, char *const b[]) {
    printf("execv is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int execve(const char *a, char *const b[], char *const c[]) {
    printf("execve is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int execvp(const char *a, char *const b[]) {
    printf("execvp is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

void  _exit(int a) {
    printf("exit is not implemented yet, WHY DO YOU USE IT ?\n");
}

int fchown(int a, uid_t b, gid_t c) {
    printf("fchown is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fchdir(int a) {
    printf("fchdir is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fdatasync(int a) {
    printf("fdatasync is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

pid_t fork() {
    printf("fork is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

long  fpathconf(int a, int b) {
    printf("fpathconf is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int fsync(int a) {
    printf("fsync is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int ftruncate(int a, off_t b) {
    printf("ftruncate is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *getcwd(char *a, size_t b) {
    printf("getcwd is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

gid_t getegid() {
    printf("getegid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

uid_t geteuid() {
    printf("geteuid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

gid_t getgid() {
    printf("getgid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int getgroups(int a, gid_t b[]) {
    printf("getgroups is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

long  gethostid() {
    printf("gethostid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int gethostname(char *a, size_t n) {
    printf("gethostname is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *getlogin() {
    printf("getlogin is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

int getlogin_r(char *a, size_t n) {
    printf("getlogin_r is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int getopt(int a, char * const b[], const char *c) {
    printf("getopt is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

pid_t getpgid(pid_t a) {
    printf("getpgid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

pid_t getpgrp() {
    printf("getpgrp is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

pid_t getpid() {
    printf("getpid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

pid_t getppid() {
    printf("getppid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

pid_t getsid(pid_t a) {
    printf("getsid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

uid_t getuid() {
    printf("getuid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *getwd(char *a) {
    printf("getwd is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

int isatty(int a) {
    printf("isatty is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int lchown(const char *a, uid_t b, gid_t c) {
    printf("lchown is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int link(const char *a, const char *b) {
    printf("link is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int lockf(int a, int b, off_t c) {
    printf("lockf is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

off_t lseek(int a, off_t b, int c) {
    printf("lseek is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int nice(int a) {
    printf("nice is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

long  pathconf(const char *a, int b) {
    printf("pathconf is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int pause() {
    printf("pause is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int pipe(int a[2]) {
    printf("pipe is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

ssize_t  pread(int a, void *b, size_t c, off_t d) {
    printf("pread is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

ssize_t  pwrite(int a, const void *b, size_t c, off_t d) {
    printf("pwrite is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

ssize_t  read(int a, void *b, size_t c) {
    printf("read is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

ssize_t  readlink(const char *restrict a, char *restrict b, size_t c) {
    printf("readlink is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int rmdir(const char *a) {
    printf("rmdir is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int setegid(gid_t a) {
    printf("setegid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int seteuid(uid_t a) {
    printf("seteuid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int setgid(gid_t a) {
    printf("setgid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int setpgid(pid_t a, pid_t b) {
    printf("setpgid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

pid_t setpgrp() {
    printf("setpgrp is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int setregid(gid_t a, gid_t b) {
    printf("setregid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int setreuid(uid_t a, uid_t b) {
    printf("setreuid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

pid_t setsid() {
    printf("setsid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int setuid(uid_t a) {
    printf("setuid is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

unsigned sleep(unsigned a) {
    printf("sleep is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

void  swab(const void *restrict a, void *restrict n, ssize_t c) {
    printf("swab is not implemented yet, WHY DO YOU USE IT ?\n");
}

int symlink(const char *a, const char *b) {
    printf("symlink is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

void  sync() {
    printf("sync is not implemented yet, WHY DO YOU USE IT ?\n");
}

long  sysconf(int a) {
    printf("sysconf is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

pid_t tcgetpgrp(int a) {
    printf("tcgetpgrp is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int tcsetpgrp(int a, pid_t b) {
    printf("tcsetpgrp is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int truncate(const char *a, off_t b) {
    printf("truncate is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *ttyname(int a) {
    printf("ttyname is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

int ttyname_r(int a, char *b, size_t c) {
    printf("ttyname_r is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

useconds_t   ualarm(useconds_t a, useconds_t b) {
    printf("ualarm is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int unlink(const char *a) {
    printf("unlink is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int usleep(useconds_t a) {
    printf("usleep is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

pid_t vfork() {
    printf("vfork is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

ssize_t  write(int a, const void *b, size_t c) {
    printf("write is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}
