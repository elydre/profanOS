#ifndef SYSTEM_H
#define SYSTEM_H

// build settings

#define KERNEL_VERSION  "0.10.4b"

#define PROCESS_MAX     20
#define RAMDISK_SECTOR  2048
#define RUN_DEFAULT     "/bin/shell.bin"

#define RATE_TIMER_TICK 1000     // cpu ticks per second
#define RATE_SCHEDULER  100      // schedule per second
#define RATE_COSMIC_RAY 0        // cosmic ray per second

#define RUN_STACK_BIN   0x4000
#define RUN_STACK_LIB   0x2000
#define PROCESS_ESP     0x4000

#define WATFUNC_ADDR    0x1ffff7
#define WATDILY_ADDR    0x1ffffb
#define MEM_BASE_ADDR   0x1fffff


// system.c
void sys_reboot();
void sys_shutdown();
void sys_stop();

int  sys_warning(char msg[]);
int  sys_error(char msg[]);
void sys_fatal(char msg[]);
void sys_interrupt(int code); // reserved cpu interrupt

void sys_cosmic_ray();

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
