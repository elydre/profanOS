#ifndef SYSTEM_H
#define SYSTEM_H

// system.c
void sys_reboot();
void sys_shutdown();
void sys_stop();

int sys_run_binary(char *fileName, int arg);

void do_nothing();

// watfunc.c
int wf_get_func_addr(int func_id);

#endif
