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

extern void init_tasking();
void powerfull_task(void (*main)(), int pid);

void task_printer();

void yield(int target_pid);
void kill_task(int target_pid);

// switch.asm
extern void switch_task(Registers *old, Registers *new);

#endif
