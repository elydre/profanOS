#ifndef SYSTEM_H
#define SYSTEM_H

// build settings

#define KERNEL_VERSION  "0.11.1b"

#define PROCESS_MAX     20
#define KERNEL_PRIORITY 5       // default kernel process priority
#define RAMDISK_SECTOR  4096
#define RUN_DEFAULT     "/bin/shell.bin"

#define RATE_TIMER_TICK 1000     // cpu ticks per second
#define RATE_SCHEDULER  100      // schedule per second
#define RATE_COSMIC_RAY 0        // cosmic ray per second

#define RUN_BIN_STACK_L 0x1000   // left stack size for binary
#define RUN_BIN_STACK_R 0x4000   // right stack size for binary

#define RUN_LIB_STACK_L 0x1000   // left stack size for library
#define RUN_LIB_STACK_R 0x4000   // right stack size for library

#define PROCESS_ESP     0x4000   // process stack size

#define WATFUNC_ADDR    0x1ffff7
#define WATDILY_ADDR    0x1ffffb
#define MEM_BASE_ADDR   0x1fffff

#define GRUBMOD_START   0x125000 // grub module start


// system.c
void sys_reboot();
void sys_shutdown();
void sys_stop();

int  sys_warning(char msg[]);
int  sys_error(char msg[]);
void sys_fatal(char msg[]);
void sys_interrupt(int code); // reserved cpu interrupt

void sys_cosmic_ray();
int sys_init_fpu();

// kshell.c
void start_kshell();
void kernel_switch_back();

// runtime.c
int run_ifexist(char path[], int argc, char **argv);

// dily.c
int dily_does_loaded(int lib_id);
int dily_get_func(int lib_id, int func_id);
int dily_load(char path[], int lib_id);
void dily_unload(int lib_id);
int dily_init();

// watfunc.c
int init_watfunc();

#endif
