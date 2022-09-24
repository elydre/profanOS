#ifndef SYSTEM_H
#define SYSTEM_H

void sys_reboot();
void sys_shutdown();
void sys_stop();

int sys_run_binary(char *fileName, int arg);

void do_nothing();

#endif
