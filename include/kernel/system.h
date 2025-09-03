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

#define KERNEL_VERSION  "1.3 beta 13"
#define KERNEL_EDITING  "dev"

#define PROCESS_MAX     64          // max process count
#define SCUBA_MAP_MAX   0x10000000  // scuba map to 256MB max
#define FS_MAX_DISKS    256         // max disk count
#define RUN_DEF_PATH    "/bin/x/rosemary.elf"
#define RUN_DEF_NAME    "rosemary"

#define RATE_TIMER_TICK 1000        // cpu ticks per second
#define RATE_SCHEDULER  100         // schedule per second

#define RUN_HEAP_ADDR   0xB0000000  // virtual base address for binary
#define RUN_HEAP_SIZE   0x1000      // virtual memory count

#define PROC_ESP_SIZE   0x20000     // process stack size
#define PROC_ESP_ADDR   RUN_HEAP_ADDR - PROC_ESP_SIZE

#define WATPOK_ADDR     0x1FFFFB
#define MEM_BASE_ADDR   0x1FFFFF


// system.c
void kernel_exit_current(void);

extern uint8_t IN_KERNEL;

void     sys_entry_kernel(void);
void     sys_exit_kernel(int restore_pic);
uint32_t sys_get_kernel_time(void);

extern char sys_safe_buffer[256];
int sys_set_reporter(int (*reporter)(char *));

void sys_reboot(void);
void sys_shutdown(void);
int  sys_power(int action);
void sys_nothing_todo(void);

void sys_warning(char *msg, ...);
void sys_error(char *msg, ...);
void sys_interrupt(uint8_t code, int err_code); // reserved for ISRs

int   sys_init(void);
char *sys_kinfo(char *buffer, int size);

const char *sys_addr2name(uint32_t addr);
uint32_t    sys_name2addr(const char *name);

// runtime.c
int elf_exec(uint32_t sid, char **argv, char **envp);
int elf_start(char *file, int sleep, char **argv, int *pid_ptr);

// pok.c
int      mod_init(void);
int      mod_load(char *path, uint32_t lib_id);
int      mod_unload(uint32_t lib_id);
uint32_t mod_get_func(uint32_t lib_id, uint32_t func_id);

#define sys_fatal(msg, ...) sod_fatal(__FILE__, __LINE__, msg, ##__VA_ARGS__)
void sod_fatal(char *file_name, int line, char *msg, ...);

#endif
