#ifndef SYSTEM_H
#define SYSTEM_H

#define VERSION "0.5.4"

// system.c
void sys_reboot();
void sys_shutdown();
void sys_stop();

int sys_run_binary(char *path, int arg);
int sys_run_ifexist(char path[], int arg);


void do_nothing();

void sys_warning(char msg[]);
void sys_error(char msg[]);
void sys_fatal(char msg[]);
void sys_interrupt(int code);

// watfunc.c
int wf_get_func_addr(int func_id);

// kshell.c
void start_kshell();

#endif
