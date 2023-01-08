#ifndef TASK_H
#define TASK_H

#include <type.h>

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} proc_rgs_t;

typedef struct {
    proc_rgs_t regs;
    int pid, ppid;
    int state;
    uint32_t esp_addr;
    uint8_t *run_mem;
    char name[64];
} process_t;

int process_init();
void schedule();

int process_create(void (*func)(), char *name);

void process_sleep(int pid);
void process_wakeup(int pid);
void process_kill(int pid);
void process_exit();

void process_debug();

void process_set_bin_mem(int pid, uint8_t *mem);
uint8_t *process_get_bin_mem(int pid);

int process_get_ppid(int pid);
int process_get_running_pid();

// switch.asm
extern void process_asm_switch(proc_rgs_t *old, proc_rgs_t *new);

#endif
