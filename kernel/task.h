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
void create_task(Task *task, void (*main)(), uint32_t flags, uint32_t *pagedir, int pid);

void yield();
void kill_task();

extern void switch_task(Registers *old, Registers *new); // The function which actually switches

#endif
