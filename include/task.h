#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} task_rgs_t;

typedef struct {
    task_rgs_t regs;
    int pid, isdead;
} task_t;


void tasking_init();
int task_create(void (*func)());

void yield(int target_pid);
void task_kill_yield(int target_pid);
void task_kill(int target_pid);

int task_get_current_pid();
int task_get_alive();
int task_get_max();


// switch.asm
extern void task_switch(task_rgs_t *old, task_rgs_t *new);

#endif
