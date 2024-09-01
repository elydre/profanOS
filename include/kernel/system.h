/*****************************************************************************\
|   === system.h : 2024 ===                                                   |
|                                                                             |
|    Kernel main header                                            .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef SYSTEM_H
#define SYSTEM_H

// build settings

#define KERNEL_VERSION  "sc-18"
#define KERNEL_EDITING  "syscall"

#define PROCESS_MAX     20          // max process count
#define PROC_PRIORITY   2           // default process priority
#define SCUBA_MAP_MAX   0x10000000  // scuba map to 256MB max
#define FS_MAX_DISKS    256         // max disk count
#define RUN_DEFAULT     "/bin/sys/init.bin"

#define RATE_TIMER_TICK 1000        // cpu ticks per second
#define RATE_SCHEDULER  100         // schedule per second

#define RUN_BIN_VBASE   0xB0000000  // virtual base address for binary
#define RUN_BIN_VCUNT   0x10000     // virtual memory count

#define DILY_MAX        128         // max dily loaded library
#define RUN_LIB_STACK   0x1000      // left stack size for library

#define PROC_ESP_SIZE   0x10000     // process stack size
#define PROC_ESP_ADDR   RUN_BIN_VBASE - PROC_ESP_SIZE

#define WATDILY_ADDR    0x1FFFFB
#define MEM_BASE_ADDR   0x1FFFFF


// system.c
void kernel_exit_current(void);

extern uint8_t IN_KERNEL;

int sys_entry_kernel(int tolerate_error);
int sys_exit_kernel(int tolerate_error);

extern char sys_safe_buffer[256];
int sys_set_reporter(int (*reporter)(char *));

void sys_reboot(void);
void sys_shutdown(void);
int  sys_power(int action);

void sys_warning(char *msg, ...);
void sys_error(char *msg, ...);
void sys_interrupt(uint8_t code, int err_code); // reserved cpu interrupt

int   sys_init(void);
char *sys_kinfo(void);

// runtime.c
int run_ifexist(char *file, int sleep, char **argv, int *pid_ptr);
int binary_exec(uint32_t sid, int argc, char **argv, char **envp);
int force_exit_pid(int pid, int ret_code, int warn_leaks);

// dily.c
int      dily_does_loaded(uint32_t lib_id);
int      dily_load(char *path, uint32_t lib_id);
int      dily_unload(uint32_t lib_id);
uint32_t dily_get_func(uint32_t lib_id, uint32_t func_id);

#define sys_fatal(msg, ...) sod_fatal(__FILE__, __LINE__, msg, ##__VA_ARGS__)
void sod_fatal(char *file_name, int line, char *msg, ...);

#endif
