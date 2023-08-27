#ifndef KTYPE_H
#define KTYPE_H

#ifndef FS_SECTOR_SIZE
#define FS_SECTOR_SIZE 256
#endif

typedef struct {
    int seconds;
    int minutes;
    int hours;
    int day_of_week;
    int day_of_month;
    int month;
    int year;
    int full[6];
} i_time_t;

#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

#ifndef NULL
#define NULL 0
#endif

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef struct {
    uint8_t data[FS_SECTOR_SIZE];  // sector data
} sector_t;

typedef struct {
    sector_t *sectors;          // first sector pointer
    uint32_t size;              // sector count

    uint8_t *used;              // array sectors (bool)
    uint32_t *free;             // array of free sector ids
    uint32_t used_count;        // used sector count
} vdisk_t;

typedef struct {
    uint32_t device;            // device id
    uint32_t sector;            // sector id
} sid_t;

typedef struct {
    vdisk_t **vdisk;            // list mounted virtual disks
    uint32_t vdisk_count;       // virtual disk count
} filesys_t;

typedef struct {
    char *path;     // path to file
    sid_t sid;      // sector id (can be null)
    
    int argc;       // argument count
    char **argv;    // argument list

    uint32_t vbase; // virtual base address
    uint32_t vcunt; // virtual count

    uint32_t stack; // stack size

    uint8_t sleep; // sleep after start
} runtime_args_t;

#endif
