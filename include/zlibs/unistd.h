#ifndef UNISTD_ID
#define UNISTD_ID 1014

#include <type.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define access ((int (*)(const char *, int)) get_func_addr(UNISTD_ID, 2))
#define alarm ((unsigned int (*)(unsigned int)) get_func_addr(UNISTD_ID, 3))
#define chdir ((int (*)(const char *)) get_func_addr(UNISTD_ID, 4))
#define chown ((int (*)(const char *, uid_t, gid_t)) get_func_addr(UNISTD_ID, 5))
#define close ((int (*)(int)) get_func_addr(UNISTD_ID, 6))
#define confstr ((size_t (*)(int, char *, size_t)) get_func_addr(UNISTD_ID, 7))
#define crypt ((char *(*)(char *, const char *)) get_func_addr(UNISTD_ID, 8))
#define ctermid ((char *(*)(char *)) get_func_addr(UNISTD_ID, 9))
#define dup ((int (*)(int)) get_func_addr(UNISTD_ID, 10))
#define dup2 ((int (*)(int, int)) get_func_addr(UNISTD_ID, 11))
#define encrypt ((void (*)(char *, int)) get_func_addr(UNISTD_ID, 12))
#define execl ((int (*)(const char *, const char *, ...)) get_func_addr(UNISTD_ID, 13))
#define execle ((int (*)(const char *, const char *, ...)) get_func_addr(UNISTD_ID, 14))
#define execlp ((int (*)(const char *, const char *, ...)) get_func_addr(UNISTD_ID, 15))
#define execv ((int (*)(const char *, char *const[])) get_func_addr(UNISTD_ID, 16))
#define execve ((int (*)(const char *, char *const[], char *const[])) get_func_addr(UNISTD_ID, 17))
#define execvp ((int (*)(const char *, char *const[])) get_func_addr(UNISTD_ID, 18))
#define _exit ((void (*)(int)) get_func_addr(UNISTD_ID, 19))
#define fchown ((int (*)(int, uid_t, gid_t)) get_func_addr(UNISTD_ID, 20))
#define fchdir ((int (*)(int)) get_func_addr(UNISTD_ID, 21))
#define fdatasync ((int (*)(int)) get_func_addr(UNISTD_ID, 22))
#define fork ((pid_t (*)(void)) get_func_addr(UNISTD_ID, 23))
#define fpathconf ((long (*)(int, int)) get_func_addr(UNISTD_ID, 24))
#define fsync ((int (*)(int)) get_func_addr(UNISTD_ID, 25))
#define ftruncate ((int (*)(int, off_t)) get_func_addr(UNISTD_ID, 26))
#define getcwd ((char *(*)(char *, size_t)) get_func_addr(UNISTD_ID, 27))
#define getegid ((gid_t (*)(void)) get_func_addr(UNISTD_ID, 28))
#define geteuid ((uid_t (*)(void)) get_func_addr(UNISTD_ID, 29))
#define getgid ((gid_t (*)(void)) get_func_addr(UNISTD_ID, 30))
#define getgroups ((int (*)(int, gid_t *)) get_func_addr(UNISTD_ID, 31))
#define gethostid ((long (*)(void)) get_func_addr(UNISTD_ID, 32))
#define gethostname ((int (*)(char *, size_t)) get_func_addr(UNISTD_ID, 33))
#define getlogin ((char *(*)(void)) get_func_addr(UNISTD_ID, 34))
#define getlogin_r ((int (*)(char *, size_t)) get_func_addr(UNISTD_ID, 35))
#define getopt ((int (*)(int, char *const[], const char *)) get_func_addr(UNISTD_ID, 36))
#define getpgid ((pid_t (*)(pid_t)) get_func_addr(UNISTD_ID, 37))
#define getpgrp ((pid_t (*)(void)) get_func_addr(UNISTD_ID, 38))
#define getpid ((pid_t (*)(void)) get_func_addr(UNISTD_ID, 39))
#define getppid ((pid_t (*)(void)) get_func_addr(UNISTD_ID, 40))
#define getsid ((pid_t (*)(pid_t)) get_func_addr(UNISTD_ID, 41))
#define getuid ((uid_t (*)(void)) get_func_addr(UNISTD_ID, 42))
#define getwd ((char *(*)(char *)) get_func_addr(UNISTD_ID, 43))
#define isatty ((int (*)(int)) get_func_addr(UNISTD_ID, 44))
#define lchown ((int (*)(const char *, uid_t, gid_t)) get_func_addr(UNISTD_ID, 45))
#define link ((int (*)(const char *, const char *)) get_func_addr(UNISTD_ID, 46))
#define lockf ((int (*)(int, int, off_t)) get_func_addr(UNISTD_ID, 47))
#define lseek ((off_t (*)(int, off_t, int)) get_func_addr(UNISTD_ID, 48))
#define nice ((int (*)(int)) get_func_addr(UNISTD_ID, 49))
#define pathconf ((long (*)(const char *, int)) get_func_addr(UNISTD_ID, 50))
#define pause ((int (*)(void)) get_func_addr(UNISTD_ID, 51))
#define pipe ((int (*)(int *)) get_func_addr(UNISTD_ID, 52))
#define pread ((ssize_t (*)(int, void *, size_t, off_t)) get_func_addr(UNISTD_ID, 53))
#define pwrite ((ssize_t (*)(int, const void *, size_t, off_t)) get_func_addr(UNISTD_ID, 54))
#define read ((ssize_t (*)(int, void *, size_t)) get_func_addr(UNISTD_ID, 55))
#define readlink ((ssize_t (*)(const char *, char *, size_t)) get_func_addr(UNISTD_ID, 56))
#define rmdir ((int (*)(const char *)) get_func_addr(UNISTD_ID, 57))
#define setegid ((int (*)(gid_t)) get_func_addr(UNISTD_ID, 58))
#define seteuid ((int (*)(uid_t)) get_func_addr(UNISTD_ID, 59))
#define setgid ((int (*)(gid_t)) get_func_addr(UNISTD_ID, 60))
#define setpgid ((int (*)(pid_t, pid_t)) get_func_addr(UNISTD_ID, 61))
#define setpgrp ((pid_t (*)(void)) get_func_addr(UNISTD_ID, 62))
#define setregid ((int (*)(gid_t, gid_t)) get_func_addr(UNISTD_ID, 63))
#define setreuid ((int (*)(uid_t, uid_t)) get_func_addr(UNISTD_ID, 64))
#define setsid ((pid_t (*)(void)) get_func_addr(UNISTD_ID, 65))
#define setuid ((int (*)(uid_t)) get_func_addr(UNISTD_ID, 66))
#define sleep ((unsigned int (*)(unsigned int)) get_func_addr(UNISTD_ID, 67))
#define swab ((void (*)(const void *, void *, ssize_t)) get_func_addr(UNISTD_ID, 68))
#define symlink ((int (*)(const char *, const char *)) get_func_addr(UNISTD_ID, 69))
#define sync ((void (*)(void)) get_func_addr(UNISTD_ID, 70))
#define sysconf ((long (*)(int)) get_func_addr(UNISTD_ID, 71))
#define tcgetpgrp ((pid_t (*)(int)) get_func_addr(UNISTD_ID, 72))
#define tcsetpgrp ((int (*)(int, pid_t)) get_func_addr(UNISTD_ID, 73))
#define truncate ((int (*)(const char *, off_t)) get_func_addr(UNISTD_ID, 74))
#define ttyname ((char *(*)(int)) get_func_addr(UNISTD_ID, 75))
#define ttyname_r ((int (*)(int, char *, size_t)) get_func_addr(UNISTD_ID, 76))
#define ualarm ((useconds_t (*)(useconds_t, useconds_t)) get_func_addr(UNISTD_ID, 77))
#define unlink ((int (*)(const char *)) get_func_addr(UNISTD_ID, 78))
#define usleep ((int (*)(useconds_t)) get_func_addr(UNISTD_ID, 79))
#define vfork ((pid_t (*)(void)) get_func_addr(UNISTD_ID, 80))
#define write ((ssize_t (*)(int, const void *, size_t)) get_func_addr(UNISTD_ID, 81))

#endif
