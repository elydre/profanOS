#ifndef TASK_H
#define TASK_H

#include <type.h>

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} proc_rgs_t;

typedef struct {
    proc_rgs_t regs;
    int pid, state;
    uint32_t esp_addr;
    uint8_t *bin_mem;
    char name[64];
} process_t;

int process_init();
int process_create(void (*func)(), char *name);

int process_kill(int pid);
void schedule();

int process_get_current_pid();

// switch.asm
extern void process_asm_switch(proc_rgs_t *old, proc_rgs_t *new);

#endif
