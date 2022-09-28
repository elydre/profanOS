#ifndef SYSTEM_H
#define SYSTEM_H

// system.c
void sys_reboot();
void sys_shutdown();
void sys_stop();

int sys_run_binary(char *fileName, int arg);

void do_nothing();

void sys_warning(int code, char msg[]);
void sys_error(int code, char msg[]);
void sys_fatal(int code, char msg[]);
void sys_interrupt(int code);

// watfunc.c
int wf_get_func_addr(int func_id);

#endif
