#ifndef TASK_H
#define TASK_H

#include <stdint.h>


extern void initTasking();

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} Registers;

typedef struct Task {
    Registers regs;
    int pid;
} Task;

extern void initTasking();
void createTask(Task *task, void (*main)(), uint32_t flags, uint32_t *pagedir, int pid);

void yield(char func_name[]) ; // Switch task frontend
extern void switchTask(Registers *old, Registers *new); // The function which actually switches

#endif
