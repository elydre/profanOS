/*****************************************************************************\
|   === types.h : 2024 ===                                                    |
|                                                                             |
|    Implementation of the sys/types.h header file from libC       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <stdint.h>

typedef int32_t  blkcnt_t;    // file block counts
typedef int32_t  blksize_t;   // block sizes
typedef int32_t  clock_t;     // system times in clock ticks or CLOCKS_PER_SEC
typedef int32_t  clockid_t;   // clock ID type in the clock and timer functions
typedef uint32_t dev_t;       // device IDs
typedef uint32_t fsblkcnt_t;  // file system block counts
typedef uint32_t fsfilcnt_t;  // file system file counts
typedef uint32_t gid_t;       // group IDs
typedef uint32_t id_t;        // general identifier
typedef uint32_t ino_t;       // file serial numbers
typedef int32_t  key_t;       // interprocess communication
typedef uint32_t mode_t;      // some file attributes
typedef uint32_t nlink_t;     // link counts
typedef int32_t  off_t;       // file sizes
typedef int32_t  pid_t;       // process IDs and process group IDs
typedef int32_t  ssize_t;     // a count of bytes or an error indication
typedef int32_t  suseconds_t; // time in microseconds
typedef int32_t  time_t;      // time in seconds
typedef uint32_t timer_t;     // timer ID returned by timer_create()
typedef uint32_t uid_t;       // user IDs
typedef uint32_t useconds_t;  // time in microseconds

typedef __SIZE_TYPE__  size_t; // sizes of objects

struct _TYPEDEF; // opaque structure for pthreads (not implemented)

typedef struct _TYPEDEF pthread_attr_t;       // identify a thread attribute object
typedef struct _TYPEDEF pthread_cond_t;       // condition variables
typedef struct _TYPEDEF pthread_condattr_t;   // identify a condition attribute object
typedef int32_t pthread_key_t;                // thread-specific data keys
typedef struct _TYPEDEF pthread_mutex_t;      // mutexes
typedef struct _TYPEDEF pthread_mutexattr_t;  // identify a mutex attribute object
typedef uint32_t pthread_once_t;              // dynamic package initialisation
typedef struct _TYPEDEF pthread_rwlock_t;     // read-write locks
typedef struct _TYPEDEF pthread_rwlockattr_t; // read-write lock attributes
typedef uint32_t pthread_t;                   // identify a thread

#endif
