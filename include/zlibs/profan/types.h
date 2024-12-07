/*****************************************************************************\
|   === types.h : 2024 ===                                                    |
|                                                                             |
|    Kernel types and structures (and signal definitions)          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_TYPES_H
#define _PROFAN_TYPES_H

#include <stdint.h>

typedef struct {
    uint8_t data[512];      // sector data
} fs_sector_t;

typedef struct {
    fs_sector_t *sectors;   // first sector pointer
    uint32_t size;          // sector count

    uint8_t *used;          // array sectors (bool)
    uint32_t *free;         // array of free sector ids
    uint32_t used_count;    // used sector count
} fs_vdisk_t;

typedef struct {
    fs_vdisk_t **vdisk;     // list mounted virtual disks
    uint32_t vdisk_count;   // virtual disk count
} filesys_t;

typedef struct {
    char *path;     // path to file

    int argc;       // argument count
    char **argv;    // argument list
    char **envp;    // environment list

    uint8_t sleep_mode;  // sleep mode
} runtime_args_t;


/************* Signal handling *************/

union sigval {
    int sival_int;   // Integer value
    void *sival_ptr; // Pointer value
};

typedef struct sigevent {
    int    sigev_notify;      // Notification method
    int    sigev_signo;       // Notification signal
    union sigval sigev_value; // Data passed with notification

    // Function used for thread notification (SIGEV_THREAD)
    void (*sigev_notify_function)(union sigval);

    // Attributes for notification thread (SIGEV_THREAD)
    void  *sigev_notify_attributes;
} sigevent_t;

#endif
