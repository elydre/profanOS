#ifndef SYSTEM_H
#define SYSTEM_H

#define VERSION "FSv2-07"
#define WATFUNC_ADDR 0x199990

// system.c
void sys_reboot();
void sys_shutdown();
void sys_stop();

int  sys_warning(char msg[]);
int  sys_error(char msg[]);
void sys_fatal(char msg[]);
void sys_interrupt(int code);

int sys_get_setting(char name[]);

// kmenu.c
void task_menu();

// kshell.c
void start_kshell();

// runtime.c
int run_binary(char path[], int silence, int argc, char **argv);
int run_ifexist(char path[], int argc, char **argv);

// watfunc.c
int wf_get_func_addr(int func_id);
void init_watfunc();

#endif
