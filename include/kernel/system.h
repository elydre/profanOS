#ifndef SYSTEM_H
#define SYSTEM_H

#define VERSION "dily-08"


#define WATFUNC_ADDR 0x199990
#define WATDILY_ADDR 0x199994


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
void dily_load(char path[], int lib_id);
int dily_get_func(int lib_id, int func_id);

// watfunc.c
int wf_get_func_addr(int func_id);
void init_watfunc();

#endif
