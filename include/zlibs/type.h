#ifndef TYPE_H
#define TYPE_H

/*****************************
 *                          *
 *       kernel types       *
 *                          *
*****************************/

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
    uint32_t max_disks;         // maximum virtual disk count
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

/*****************************
 *                          *
 *        libs types        *
 *                          *
*****************************/

typedef struct _div_t {
    int quot;
    int rem;
} div_t;

typedef struct _ldiv_t {
    long quot;
    long rem;
} ldiv_t;

typedef struct _lldiv_t {
    long long quot;
    long long rem;
} lldiv_t;

struct drand48_data {
    unsigned short __x[3]; /* Current state.  */
    unsigned short __old_x[3]; /* Old state.  */
    unsigned short __c; /* Additive const. in congruential formula.  */
    unsigned short __init; /* Flag for initializing.  */
    unsigned long long __a; /* Factor in congruential formula.  */
};

// more things for stdlib :
typedef unsigned int size_t;
typedef unsigned short wchar_t;

typedef struct _Mbstatet
{ // state of a multibyte translation
    unsigned long _Wchar;
    unsigned short _Byte, _State;
} _Mbstatet;

typedef _Mbstatet mbstate_t;

typedef signed char        int_least8_t;
typedef short              int_least16_t;
typedef int                int_least32_t;
typedef long long          int_least64_t;
typedef unsigned char      uint_least8_t;
typedef unsigned short     uint_least16_t;
typedef unsigned int       uint_least32_t;
typedef unsigned long long uint_least64_t;

typedef signed char        int_fast8_t;
typedef int                int_fast16_t;
typedef int                int_fast32_t;
typedef long long          int_fast64_t;
typedef unsigned char      uint_fast8_t;
typedef unsigned int       uint_fast16_t;
typedef unsigned int       uint_fast32_t;
typedef unsigned long long uint_fast64_t;

typedef void (*oefuncp)(int, void *);  /* on_exit function pointer */
typedef int (*__compar_fn_t)(const void *, const void *);
typedef int (*__compar_d_fn_t)(const void *, const void *);

struct random_data {
    int32_t *fptr;      /* Front pointer.  */
    int32_t *rptr;      /* Rear pointer.  */
    int32_t *state;     /* Array of state values.  */
    int rand_type;      /* Type of random number generator.  */
    int rand_deg;       /* Degree of random number generator.  */
    int rand_sep;       /* Distance between front and rear.  */
    int32_t *end_ptr;   /* Pointer behind state table.  */
};

typedef void *locale_t; // PLACEHOLDER, //TODO : implement locale_t

typedef unsigned char __string_uchar_t;
typedef int            errno_t;
typedef unsigned short wctype_t;
typedef long           __time32_t;
typedef size_t rsize_t;

typedef struct FILE {
    char *filename;
    char *mode;

    int   file_pos;
    int   type;
    sid_t sid;
} FILE;

typedef struct fpos_t {

} fpos_t;

typedef unsigned long DWORD, *PDWORD, *LPDWORD;

typedef int bool;

typedef uint64_t time_t;
typedef int timer_t;
typedef int clockid_t;
typedef unsigned long clock_t;

union sigval { /* Data passed with notification */
    int sival_int;   /* Integer value */
    void *sival_ptr; /* Pointer value */
};

typedef struct sigevent_t {
    int    sigev_notify;  /* Notification method */
    int    sigev_signo;   /* Notification signal */
    union sigval sigev_value; /* Data passed with notification */
    /* Function used for thread
    notification (SIGEV_THREAD) */
    void (*sigev_notify_function)(union sigval);
    /* Attributes for notification thread
    (SIGEV_THREAD) */
    void  *sigev_notify_attributes;
} sigevent_t;

#define LDOUBLE long double
#define LLONG long long

typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef int pid_t;
typedef int off_t;
typedef int ssize_t;
typedef unsigned int useconds_t;

#endif
