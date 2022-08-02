#ifndef TASK_H
#define TASK_H

#include <stdint.h>


extern void initTasking();

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} Registers;

typedef struct Task {
    Registers regs;
    char name[32];
} Task;

extern void initTasking();
void createTask(Task *task, void (*main)(), uint32_t flags, uint32_t *pagedir, char name[]);

extern void yield(); // Switch task frontend
extern void switchTask(Registers *old, Registers *new); // The function which actually switches

#endif
