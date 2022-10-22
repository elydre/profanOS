#ifndef SYSTEM_H
#define SYSTEM_H

#define VERSION "TT-10"
#define FUNC_ADDR_SAVE 0x199990

// system.c
void sys_reboot();
void sys_shutdown();
void sys_stop();

int sys_run_binary(char path[], int silence, int argc, char **argv);
int sys_run_ifexist(char path[], int argc, char **argv);

int sys_warning(char msg[]);
int sys_error(char msg[]);
void sys_fatal(char msg[]);
void sys_interrupt(int code);

int sys_get_setting(char name[]);

// watfunc.c
int wf_get_func_addr(int func_id);
void init_watfunc();

// kshell.c
void start_kshell();

#endif
