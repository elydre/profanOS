#ifndef SYSTEM_H
#define SYSTEM_H

// build settings

#define KERNEL_VERSION  "1.1.6c"
#define KERNEL_EDITING  "generic"

#define PROCESS_MAX     20          // max process count
#define PROC_PRIORITY   2           // default process priority
#define SCUBA_MAP_TO    0x7800000   // scuba map to 120MB
#define FS_MAX_DISKS    256         // max disk count
#define RUN_DEFAULT     "/bin/tools/init.bin"

#define RATE_TIMER_TICK 1000        // cpu ticks per second
#define RATE_SCHEDULER  100         // schedule per second

#define RUN_BIN_VBASE   0xC0000000  // virtual base address for binary
#define RUN_BIN_VCUNT   0x10000     // virtual memory count

#define DILY_MAX        128         // max dily loaded library
#define RUN_LIB_STACK   0x1000      // left stack size for library

#define PROCESS_ESP     0x10000     // process stack size

#define WATFUNC_ADDR    0x1ffff7
#define WATDILY_ADDR    0x1ffffb
#define MEM_BASE_ADDR   0x1fffff


// system.c
extern char sys_safe_buffer[256];
void sys_set_reporter(int (*reporter)(char *));

void sys_reboot(void);
void sys_shutdown(void);

void sys_warning(char *msg, ...);
void sys_error(char *msg, ...);
void sys_interrupt(int code, int err_code); // reserved cpu interrupt

int   sys_init(void);
char *sys_kinfo(void);

// kshell.c
void start_kshell(void);
void kernel_switch_back(void);
void kernel_exit_current(void);

// runtime.c
int run_ifexist(char *file, int sleep, char **argv, int *pid_ptr);
int binary_exec(sid_t sid, uint32_t vcunt, char **argv);
int force_exit_pid(int pid, int ret_code, int warn_leaks);

// dily.c
int      dily_does_loaded(uint32_t lib_id);
int      dily_load(char *path, uint32_t lib_id);
int      dily_unload(uint32_t lib_id);
uint32_t dily_get_func(uint32_t lib_id, uint32_t func_id);

// watfunc.c
int init_watfunc(void);

#define sys_fatal(msg, ...) sod_fatal(__FILE__, __LINE__, msg, ##__VA_ARGS__)
void sod_fatal(char *file_name, int line, char *msg, ...);

#endif
