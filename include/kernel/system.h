#ifndef SYSTEM_H
#define SYSTEM_H

// build settings

#define KERNEL_VERSION  "dily-21"

#define TASK_MAX_COUNT  20
#define RAMDISK_SECTOR  2048
#define RUN_DEFAULT     "/bin/shell.bin"

#define RUN_STACK_BIN   0x2000
#define RUN_STACK_LIB   0x2000
#define TASK_ESP_ALLOC  0x2000

#define WATFUNC_ADDR    0x199990
#define WATDILY_ADDR    0x199994
#define MEM_BASE_ADDR   0x200000


// system.c
void sys_reboot();
void sys_shutdown();
void sys_stop();

int  sys_warning(char msg[]);
int  sys_error(char msg[]);
void sys_fatal(char msg[]);
void sys_interrupt(int code); // reserved cpu interrupt

// kmenu.c
void task_menu();

// kshell.c
void start_kshell();

// runtime.c
int run_binary(char path[], int silence, int argc, char **argv);
int run_ifexist(char path[], int argc, char **argv);

// dily.c
int dily_does_loaded(int lib_id);
int dily_get_func(int lib_id, int func_id);
void dily_load(char path[], int lib_id);
void dily_unload(int lib_id);
void dily_init();

// watfunc.c
void init_watfunc();

#endif
