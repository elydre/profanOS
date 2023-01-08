#ifndef TASK_H
#define TASK_H

#include <type.h>

#define PROCESS_RUNNING  0
#define PROCESS_WAITING  1
#define PROCESS_SLEEPING 2
#define PROCESS_KILLED   3
#define PROCESS_DEAD     4

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} proc_rgs_t;

typedef struct {
    proc_rgs_t regs;
    int pid, ppid, state;
    uint32_t esp_addr;
    uint8_t *run_mem;
    void *custom;
    char name[64];
} process_t;

// kenel reserved
int process_init();
void schedule();

// runtime reserved
void process_set_bin_mem(int pid, uint8_t *mem);
uint8_t *process_get_bin_mem(int pid);

// process control
int process_create(void (*func)(), char *name);

void process_sleep(int pid);
void process_wakeup(int pid);
void process_kill(int pid);
void process_exit();

// get process info
int process_get_running_pid();
int process_get_ppid(int pid);
int process_generate_pid_list(int *list, int max);
int process_get_name(int pid, char *name);
int process_get_state(int pid);

void *process_get_custom(int pid);
void process_set_custom(int pid, void *custom);

// switch.asm
extern void process_asm_switch(proc_rgs_t *old, proc_rgs_t *new);

#endif
