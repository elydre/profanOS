#ifndef TASK_H
#define TASK_H

#include <stdint.h>


typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} Registers;

typedef struct Task {
    Registers regs;
    int pid;
    int isdead;
} Task;

extern void tasking_init();
void task_powerfull(void (*main)(), int pid);

void task_print();

void yield(int target_pid);
void task_kill_yield(int target_pid);

// switch.asm
extern void task_switch(Registers *old, Registers *new);

#endif
